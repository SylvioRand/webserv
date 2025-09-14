/* ************************************************************************** */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 10:26:47 by srandria          #+#    #+#             */
/*   Updated: 2025/08/21 13:10:29 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/Server.hpp"

Server::Server(const Config& config) : _config(config)
{
  this->loadMimeTypes();
  this->start_server_();
}

Server::~Server(void)
{
}

void Server::start_server_(void)
{
  logger(LOG_INFO, "Server is starting");
  signal(SIGINT, signalHandler);
  signal(SIGQUIT, signalHandler);
  this->create_all_listeners_();
  static bool isClientClosed;

  while (true)
  {
    int ready = poll(&_pool_fds[0], _pool_fds.size(), 20);
    this->handleServerTimeout();
    this->handleFinishedChildren();
    if (this->checkShutdownRequest())
      return ;
    if (ready == -1)
      throwWithLog(LOG_FATAL, "poll() failed");


    for (int i = static_cast<int>(_pool_fds.size()) - 1; i >= 0 && ready > 0; --i)
    {
      if (_pool_fds[i].revents == 0)
        continue;

      --ready;
      int fd = _pool_fds[i].fd;
      short revents = _pool_fds[i].revents;

      if (std::find(_listener_fds.begin(), _listener_fds.end(), fd) != _listener_fds.end())
      {
        if (revents & POLLIN)
          this->accept_new_client_(fd);
      }
      else if (_clients.find(fd) != _clients.end())
      {
        isClientClosed = false;
        if (revents & POLLIN)
          this->handle_pollin_(fd, isClientClosed);
        if (!isClientClosed && revents & POLLOUT)
          this->handle_pollout_(fd, isClientClosed);
        if (!isClientClosed && revents & (POLLERR | POLLHUP | POLLNVAL))
          this->close_client_(fd);
      }
      else if (std::find(_pipeFd.begin(), _pipeFd.end(), fd) != _pipeFd.end())
      {
        if (revents & (POLLIN | POLLHUP))
        {
          this->readCgiResponse(fd, this->_pipeFdClient[fd]);
        }
        if (revents & POLLOUT)
        {
          this->sendRequestBodyToCgi(fd, this->_pipeFdClient[fd]);
        }
      }
      else
        this->close_client_(fd);
    }
  }
}

void  Server::handleServerTimeout(void)
{
  for (std::vector<struct pollfd>::iterator it = _pool_fds.begin(); it != _pool_fds.end(); ++it)
  {
    if (this->_clients.find(it->fd) != this->_clients.end())
    {
      Client *client = this->_clients.at(it->fd);
      if (it->events == POLLIN)
      {
        if (client->getRequest().getMethod().empty() && client->getRequest()._isReadingRequest)
          this->handleClientHeaderTimeout(it->fd);
        else if (!client->getRequest()._isReadingRequest)
          this->handleKeepAliveTimeout(it->fd);
        else
          this->handleClientBodyTimeout(it->fd);
      }
      else if (it->events == POLLOUT)
        this->handleSendTimeout(it->fd);
    }
    if (this->_clients.size() == 0)
      return ;
  }
}

void  Server::handleClientHeaderTimeout(const int& fd)
{
  time_t now = time(NULL);
  double  elapsed_seconds = difftime(now, this->_clients[fd]->getLastActivity());
  if (elapsed_seconds  >= timeouts::CLIENT_HEADER_TIMEOUT)
  {
    logger(LOG_INFO, "Header timeout: closing client " + toString(fd));
    this->respond408RequestTimeout(fd);
  }
}

void  Server::handleClientBodyTimeout(const int& fd)
{
  time_t now = time(NULL);
  double  elapsed_seconds = difftime(now, this->_clients[fd]->getLastActivity());
  if (elapsed_seconds >= timeouts::CLIENT_BODY_TIMEOUT)
  {
    logger(LOG_INFO, "Body timeout: closing client " + toString(fd));
    this->respond408RequestTimeout(fd);
  }
}

void  Server::handleKeepAliveTimeout(const int& fd)
{
  time_t now = time(NULL);
  double  elapsed_seconds = difftime(now, this->_clients[fd]->getLastActivity());
  if (elapsed_seconds >= timeouts::KEEPALIVE_TIMEOUT)
  {
    logger(LOG_INFO, "Keep-alive timeout: closing client " + toString(fd));
    close_client_(fd);
  }
}

void Server::handleSendTimeout(const int& fd)
{
  time_t now = time(NULL);
  double  elapsed_seconds = difftime(now, this->_clients[fd]->getLastActivity());
  if (elapsed_seconds >= timeouts::SEND_TIMEOUT)
  {
    pid_t childPid = this->_clients[fd]->getChildPid();

    if (childPid > 0)
    {
      logger(LOG_WARNING, "Send timeout: closing client " + toString(fd) +
          " after " + toString(elapsed_seconds) + "s (limit: " +
          toString(timeouts::SEND_TIMEOUT) + "s)");

      kill(childPid, SIGKILL);
    }

    if (!this->_clients[fd]->getRequest()._isCgiRequest)
    {
      logger(LOG_WARNING, "Send timeout: killing child process " + toString(childPid));
      close_client_(fd);
    }
  }
}

void  Server::handleFinishedChildren(void)
{
    int status;
    pid_t finished;

    while ((finished = waitpid(-1, &status, WNOHANG)) > 0)
    {
        for (std::map<int, Client*>::iterator it = this->_clients.begin();
             it != this->_clients.end(); ++it)
        {
            if (it->second->getChildPid() == finished)
            {
                if (WIFEXITED(status))
                {
                    logger(LOG_INFO, "Child " + toString(finished) +
                                     " exited with code " + toString(WEXITSTATUS(status)));
                    it->second->setChildPid(0);
                } else if (WIFSIGNALED(status)) {
                    logger(LOG_INFO, "Child " + toString(finished) +
                                     " killed by signal " + toString(WTERMSIG(status)));
                    close_client_(it->first);
                }
                break;
            }
        }
    }
}

void  Server::respond408RequestTimeout(const int& fd)
{
  logger(LOG_DEBUG, "In function respond408RequestTimeout");
    std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(408, fd);
  if (this->hasCustomErrorPage(408, fd))
    this->saveErrorBodyFilePath(408, fd, contentType, contentLength);
  else
  {
    body = "408 Request Timeout";
    contentLength = toString(body.size());
  }

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 408 Request Timeout\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}

void  Server::stop_server(void)
{
  logger(LOG_INFO, "Server shutting down... (Made with 💜 • Light & fast @ 42-Antananarivo)");
  std::map<int, Client*>::iterator it = this->_clients.begin();
  for (; it != this->_clients.end(); it++)
  {
    delete it->second;
  }
}

bool  Server::checkShutdownRequest(void)
{
  if (g_shouldStop == 1)
  {
    this->stop_server();
    return (true);
  }
  return (false);
}

void  Server::create_all_listeners_(void)
{
  logger(LOG_INFO, "Create all listeners");
  const std::vector<ServerConfig>& servers = this->getServers();

  for (std::vector<ServerConfig>::const_iterator it = servers.begin(); it != servers.end(); ++it)
  {
    const ServerConfig cfg = *it;

    int fd = this->createTcpSocket_();  
    this->setSocketReuseAddr_(fd);

    struct sockaddr_in  addr;

    this->buildIpv4Sockaddr_(addr, cfg);
    this->bindSocket_(fd, cfg, addr);
    this->startListener_(fd, it);
    this->addFdToPoll_(fd);
  }
}

int   Server::createTcpSocket_(void)
{
  int fd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET for ipv4
                                            // SOCK_STREAM for a reliable TCP connection.
                                            // Using 0 selects the default protocol (TCP here).
  if (fd == -1)
    throwWithLog(LOG_FATAL, "socket() failed");
  return (fd);
}

void  Server::bindSocket_(int fd, const ServerConfig &cfg, struct sockaddr_in& addr)
{
  std::ostringstream oss;

  if (cfg.host.empty() || inet_addr(cfg.host.c_str()) == INADDR_NONE)
  {
      close(fd);
      throwWithLog(LOG_FATAL, "Invalid host: {" + cfg.host + "}");
  }
  if (bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)))
  {
    close(fd);
    oss << "bind() failed : " << cfg.host << ":" << cfg.port;
    throwWithLog(LOG_FATAL, oss.str());
  }
}

void  Server::buildIpv4Sockaddr_(struct sockaddr_in& addr, const ServerConfig& cfg)
{
  std::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(cfg.port);
  addr.sin_addr.s_addr = inet_addr(cfg.host.c_str());
}

void  Server::setSocketReuseAddr_(int fd)
{
  int opt = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    throwWithLog(LOG_FATAL, "setsockopt() failed");
}

void Server::startListener_(int fd, ServerConfigConstIterator cfg)
{
  std::ostringstream oss;

  if (listen(fd, 128))
  {
    close(fd);
    oss << "listen() failed : " << cfg->host << ":" << cfg->port;
    throwWithLog(LOG_FATAL, oss.str());
  }

  oss << "🟢 listening on http://" << cfg->host << ":" << cfg->port;
  logger(LOG_INFO, oss.str());

  _listener_fds.push_back(fd);
  _serverListeners[fd] = cfg;
}

void  Server::addFdToPoll_(int fd)
{
  struct pollfd pfd;

  pfd.fd = fd;
  pfd.events = POLLIN;
  pfd.revents = 0;

  _pool_fds.push_back(pfd);
}

void  Server::accept_new_client_(int listener_fd)
{
  int                 client_fd;
  struct sockaddr_in  client_address;

  socklen_t addr_len = sizeof(client_address);
  client_fd = accept(listener_fd, (struct sockaddr*)&client_address, &addr_len);

  char ip[INET_ADDRSTRLEN];

  inet_ntop(AF_INET, &client_address.sin_addr, ip, sizeof(ip));
  logger(LOG_DEBUG, std::string("accept from ") + ip);

  if (client_fd == -1)
  {
    logger(LOG_WARNING, "accept() failed");
    return ;
  }

  this->setNonBlocking_(client_fd);
  _clients[client_fd] = new Client(client_fd, this->_serverListeners[listener_fd]);
  this->addFdToPoll_(client_fd);
  logger(LOG_INFO, "New client successfully added to poll fd=[" + toString(client_fd) + "]");
}

void  Server::setNonBlocking_(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        throwWithLog(LOG_FATAL, "fcntl(F_GETFL) failed");
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        throwWithLog(LOG_FATAL, "fcntl(F_SETFL, O_NONBLOCK) failed");
}

void  Server::handle_pollin_(int fd, bool& isClientClosed)
{
  Client *client = this->_clients[fd];
  if (!(*client).readData() || (*client).getRequest().hasError())
  {
    logger(LOG_DEBUG, "Cant` read data with readData()");
    close_client_(fd);
    isClientClosed = true;
    return ;
  }
  if ((*client).isRequestComplete() && !(*client).getRequest().hasError())
  {
    this->_clients[fd]->getRequest().setServerConf(this->_clients[fd]->getServerConfig());
    std::string method = this->getMethod(fd);
    logger(LOG_DEBUG, "method = " + this->getMethod(fd));
    if (this->_clients[fd]->getRequest()._isBadRequest)
    {
      this->badRequest_(fd);
      return ;
    }
    if (!this->isHttpMethodValid_(method))
    {
      this->respondNotImplemented_(fd);
      return ;
    }
    if (this->checkShutdownRequest())
      return ;
    this->_currentLocation = this->_clients[fd]->getRequest().getLocation();
    this->setIsCGIRequest(fd);
    if (this->getCurrentLocation().path.empty())
      this->handleNoMatchingLocation_(fd);
    else if (!this->getCurrentLocation().redirect.empty())
      this->handleRedirect_(fd);
    else if (!isSupportedHttpMethod(method))
      this->methodNotAllowed_(fd);
    else if (getCurrentLocation().methods.empty())
      this->methodNotAllowed_(fd);
    else if (this->_clients[fd]->getRequest()._isCgiRequest)
      this->prepareAndLaunchCGI(fd);
    else if (method == "GET" && this->isMethodAllowedForLocation("GET"))
      this->GETMethod_(fd);
    else if (method == "POST" && this->isMethodAllowedForLocation("POST"))
    {
      this->POSTMethod_(fd);
      if (!this->_clients[fd]->getRequest()._isReadingRequest)
      {
        this->_clients[fd]->setPath(this->getCurrentLocation().upload_dir);
        this->_clients[fd]->getRequest().setClientMaxBodySize(this->getCurrentLocation().client_max_body_size);
      }
    }
    else if (method == "DELETE" && this->isMethodAllowedForLocation("DELETE"))
      this->DELETEMethod_(fd);
    else
      this->methodNotAllowed_(fd);
    if (this->checkShutdownRequest())
      return ;
  }
  else if (this->_clients[fd]->getRequest().hasError())
    logger(LOG_DEBUG, "Error has been detected");
}

std::string  Server::getUriPath_(const int& fd)
{
  std::string uriPath;
  if (_clients.find(fd) == _clients.end())
    return "";
  else
    uriPath = this->getRequestUri_(fd);
  size_t pos = uriPath.find("?");
  if (pos == std::string::npos)
    return (uriPath);
  else
    return (uriPath.substr(0, pos));
}

void  Server::saveHeaderAndBodySize(const int& fd)
{
  this->_clients[fd]->getResponse().saveHeadersAndBodySize();
}

const LocationConfig& Server::getCurrentLocation(void)
{
  return (this->_currentLocation);
}

std::string  Server::getMethod(int fd)
{
  return (this->_clients[fd]->getRequest().getMethod());
}

std::string  Server::getVersion(int fd)
{
  return (this->_clients[fd]->getRequest().getVersion());
}

const std::string& Server::getRequestUri_(const int& fd)
{
  return (this->_clients[fd]->getRequest().getPath());
}

void  Server::setStatus(int code, int fd)
{
  this->_clients[fd]->getResponse().setStatus(code);
}

int  Server::getStatus(int fd)
{
  return (this->_clients[fd]->getResponse().getStatus());
}

bool  Server::isMethodAllowedForLocation(const std::string method)
{
  std::vector<std::string>::const_iterator it = std::find(this->getCurrentLocation().methods.begin(),
        this->getCurrentLocation().methods.end(), method);
  if (it != this->getCurrentLocation().methods.end())
  {
    logger(LOG_DEBUG, "[" + method + "] is allowed in location");
    return (true);
  }
  logger(LOG_DEBUG, "[" + method + "] is not allowed in location");
  return (false);
}

bool  Server::isSupportedHttpMethod(const std::string& method)
{
    return (method == "GET" || method == "POST" || method == "DELETE");
}

bool  Server::isHttpMethodValid_(std::string method)
{
  return (method == "GET" || method == "POST" || method == "DELETE" || method == "PUT"
      || method == "HEAD" || method == "CONNECT" || method == "OPTIONS" || method == "TRACE"
      || method == "PATCH");
}

void  Server::setPollOut_(int fd)
{
  for (std::vector<struct pollfd>::iterator it = _pool_fds.begin(); it != _pool_fds.end(); ++it)
  {
    if (it->fd == fd)
    {
      it->events = POLLOUT;
      std::ostringstream oss;

      logger(LOG_INFO, "🟢 Monitoring output events (POLLOUT) on fd=" + toString(fd));
      return ;
    }
  }

  logger(LOG_ERROR,
    "Failed to monitor input events (POLLIN) on fd=" + toString(fd) + " — descriptor not found");
}

void  Server::setPollIn_(const int& fd)
{
  this->_clients[fd]->setLastActivity();
  this->_clients[fd]->getResponse().initializeState();
  this->_clients[fd]->getRequest().shiftBufferAfterRequest();
  this->_clients[fd]->clearBuffer();
  for (std::vector<struct pollfd>::iterator it = _pool_fds.begin(); it != _pool_fds.end(); ++it)
  {
    if (it->fd == fd)
    {
      it->events = POLLIN;
      std::ostringstream oss;

      logger(LOG_INFO, "🔵 Monitoring input events (POLLIN) on fd=" + toString(fd));
      return ;
    }
  }
  logger(LOG_ERROR,
    "Failed to monitor input events (POLLIN) on fd=" + toString(fd) + " — descriptor not found");
}

void  Server::handle_pollout_(int fd, bool& isClientClosed)
{
  if (this->_clients.find(fd) == this->_clients.end())
    return ;
  Client *client = this->_clients[fd];
  std::string path = client->getRequest().getLocation().upload_dir;

  if (this->_clients[fd]->getRequest()._isCgiRequest)
  {
    client->sendCgiData();
    if (client->getResponse().getCgiBytesSent()
        == client->getResponse().getCgiRespondSize())
      client->getResponse()._isFullySent = true;
  }
  else if (this->getMethod(fd) == "POST"
      && this->_clients[fd]->getRequest().isBodySizeAllowed()
      && this->directoryExists_(path)
      && !this->_clients[fd]->getRequest()._allFilesSaved)
  {
    if (this->_clients[fd]->getRequest().hasBoundary_())
      this->saveMultipartFiles(fd);
    else
      this->saveBodyToBinary(fd);
  }

  if (this->getMethod(fd) != "POST" ||
      (this->getMethod(fd) == "POST" && this->_clients[fd]->getRequest()._allFilesSaved)
      || !this->_clients[fd]->getRequest().isBodySizeAllowed()
      || (this->getMethod(fd) == "POST" && !this->directoryExists_(path)))
  {
    client->sendData();
    if (client->getResponse().areHeadersFullySent() &&
        client->getResponse().isBodyFullySent())
      client->getResponse()._isFullySent = true;
  }
  if (client->getResponse()._isFullySent)
  {
    logger(LOG_INFO, "📤 Response fully sent to client fd="
        + toString(fd) + " | Status code: "
        + toString(client->getResponse().getStatus()));
    this->_clients[fd]->getResponse().closeBodyFileStream(fd);
    if (client->getResponse().isKeepAlive())
    {
      this->setPollIn_(fd);
    }
    else
    {
      close_client_(fd);
      isClientClosed = true;
    }
  }
}

void Server::close_client_(int fd)
{
  this->_clients[fd]->getResponse().closeBodyFileStream(fd);
  logger(LOG_INFO, "Closing client connection (fd=" + toString(fd) + ")");

  std::map<int, Client*>::iterator it = _clients.find(fd);
  if (it != _clients.end())
  {
    delete it->second;
    _clients.erase(it);
  }
  close(fd);
  std::vector<struct pollfd>::iterator itPollFd = this->_pool_fds.begin();
  while (itPollFd != this->_pool_fds.end())
  {
    if (itPollFd->fd == fd)
    {
      itPollFd = this->_pool_fds.erase(itPollFd);
      logger(LOG_INFO,
        "🟢 Client fd=" + toString(fd) + " successfully removed from poll monitoring");
      return ;
    }
    else
      ++itPollFd;
  }
  logger(LOG_INFO,
    "⚠️ Failed to remove client fd=" + toString(fd) + ": not found in poll monitoring");
}

const std::map<std::string, std::string>& Server::getHeaders(int fd)
{
    return (_clients[fd]->getRequest().getHeaders());
}

void Server::saveMatchingLocation_(const int& fd, ServerConfigConstIterator& cfg)
{
  LocationConfig  best_match;
  this->setCurrentLocation(best_match);
  size_t best_length = 0;

  std::map<std::string, LocationConfig> locations = cfg->locations;
  for (std::map<std::string, LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); it++)
  {
    const std::string& path = it->first;
    if (getUriPath_(fd).substr(0, path.size()) == path && path.size() > best_length)
    {
      best_match = it->second;
      best_length = path.size();
    }
  }
  this->setCurrentLocation(best_match);
  logger(LOG_DEBUG, "matching LocationConfig found [" + best_match.path + "]");
}

const Config& Server::getConfig(void)
{
  return (_config);
}

const std::vector<ServerConfig>&  Server::getServers(void)
{
  return (this->getConfig().getServers());
}

bool  Server::isFile_(const std::string localPath) const
{
  struct stat st;
  return (stat(localPath.c_str(), &st) == 0 &&
          S_ISREG(st.st_mode));
}

bool  Server::isReadable_(const std::string localPath) const
{
  return (access(localPath.c_str(), R_OK) == 0);
}

void  Server::setCurrentLocation(LocationConfig& location)
{
  this->_currentLocation = location;
}

void  Server::GETMethod_(const int& fd)
{
  LocationConfig  location = this->getCurrentLocation();
  std::string     localPath;
  std::string     extractUri;

  logger(LOG_DEBUG, "root location -> " + this->getCurrentLocation().root);
  if (this->getCurrentLocation().root.empty())
  {
    this->respondNotFound_(fd);
    return ;
  }
  localPath = location.root + '/' + getUriPath_(fd).substr(location.path.size());
  this->_localPath = localPath;
  logger(LOG_DEBUG, "localPath [" + localPath + "]");
  if (this->isFile_(localPath))
  {
    if (this->isReadable_(localPath))
      this->processReadableFile_(fd, localPath);
    else
      this->respondFileNotReadable(fd);
  }
  else if (!directoryExists_(localPath))
    this->respondNotFound_(fd);
  else if (hasIndexDirective_())
  {
    std::string indexPath = this->getAccessibleIndexPath_(localPath);
    if (indexPath.empty())
    {
      if (this->getCurrentLocation().autoindex)
        this->buildDirectoryListing_(fd);
      else if (this->existsAtLeastOneIndexFile_(localPath))
        this->respondIndexFilesUnreadable_(fd);
      else
        this->respondNoIndexFileFound_(fd);
    }
    else
      this->serveIndexContent_(indexPath, fd);
  }
  else if (this->getCurrentLocation().autoindex)
    this->buildDirectoryListing_(fd);
  else if (!this->getCurrentLocation().autoindex)
    this->respondDirectoryListingForbidden(fd);
}


std::string Server::getFileExtension_(std::string path)
{
  std::string uri;
  size_t pos = path.find("?");
  if (pos != std::string::npos)
    uri = path.substr(0, pos);
  else
    uri = path;
  const std::string fileName = this->getFileName(path);

  size_t extStart = uri.rfind('.');
  if (extStart == std::string::npos)
    return ("");
  else
    return (uri.substr(extStart));
  return ("");
}

const std::string Server::getFileName(const std::string uriPath)
{
  size_t pos = uriPath.rfind("/");

  if (pos != std::string::npos)
    return (uriPath.substr(pos + 1));
  return (uriPath);
}

bool Server::isExecutable_(const std::string& path)
{
  struct stat st;
  if (stat(path.c_str(), &st) == 0)
  {
    if (S_ISREG(st.st_mode))
    {
      if (st.st_mode & S_IXUSR || st.st_mode & S_IXGRP || st.st_mode & S_IXOTH)
        return true;
    }
  }
  return false;
}

void  Server::respondNotExecutable(const int& fd)
{
  logger(LOG_DEBUG, "in function respondNotExecutable");
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(500, fd);
  if (this->hasCustomErrorPage(500, fd))
    this->saveErrorBodyFilePath(500, fd, contentType, contentLength);
  else
  {
    body = "CGI script is not executable.";
    contentLength = toString(body.size());
  }

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 500 Internal Server Error\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
}


void  Server::respondIndexFilesUnreadable_(const int fd)
{
  logger(LOG_DEBUG, "In function respondIndexFilesUnreadable_");
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(403, fd);
  if (this->hasCustomErrorPage(403, fd))
    this->saveErrorBodyFilePath(403, fd, contentType, contentLength);
  else
  {
    body = "403 Forbidden: No readable index file found in this directory.";
    contentLength = toString(body.size());
  }

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 403 Forbidden\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}

void  Server::respondNoIndexFileFound_(const int fd)
{
  logger(LOG_DEBUG, "In function respondNoIndexFileFound_");
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(403, fd);
  if (this->hasCustomErrorPage(403, fd))
    this->saveErrorBodyFilePath(403, fd, contentType, contentLength);
  else
  {
    body = "403 Forbidden: No index file found in this directory.";
    contentLength = toString(body.size());
  }

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 403 Forbidden.\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}

void  Server::respondWithUploadError(const int fd)
{
  logger(LOG_DEBUG, "In function respondWithUploadError");
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(500, fd);
  if (this->hasCustomErrorPage(500, fd))
    this->saveErrorBodyFilePath(500, fd, contentType, contentLength);
  else
  {
    body = "500 Internal Server Error: Upload directory does not exist";
    contentLength = toString(body.size());
  }

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 500 Internal Server Error: Upload directory does not exist\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}

void  Server::respondInternalServerError(const int& fd)
{
  logger(LOG_DEBUG, "In function respondWithUploadError");
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(500, fd);
  if (this->hasCustomErrorPage(500, fd))
    this->saveErrorBodyFilePath(500, fd, contentType, contentLength);
  else
  {
    body = "500 Internal Server Error";
    contentLength = toString(body.size());
  }

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 500 Internal Server Error\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}




void  Server::serveIndexContent_(const std::string path, const int fd)
{
  logger(LOG_DEBUG, "In function serveIndexContent_");
  std::string contentType = this->getContentTypeByFileExtension(path);

  std::string contentLength = toString(getFileSize(path));
  std::ostringstream headers;

  headers << this->getVersion(fd) << " 200 OK\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());

  this->setStatus(200, fd);
  this->setBodyFilePath(fd, path);
  this->_clients[fd]->getResponse().openAndSaveBodyFileStream(path);
  this->setBodySize(fd, getFileSize(path));
  this->setBodyFilePath(fd, path);
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}

bool  Server::hasIndexDirective_(void)
{
  if (this->getCurrentLocation().indexs.empty())
    return (false);
  return (true);
}

std::string Server::getAccessibleIndexPath_(const std::string& path)
{
  for (std::vector<std::string>::const_iterator it = this->getCurrentLocation().indexs.begin();
      it != this->getCurrentLocation().indexs.end(); it++)
  {
    std::string resultPath = path + '/' + *it;
    if (this->isFile_(resultPath) && this->isReadable_(resultPath))
      return(resultPath);
  }
  return ("");
}

bool  Server::existsAtLeastOneIndexFile_(const std::string path)
{
  for (std::vector<std::string>::const_iterator it = this->getCurrentLocation().indexs.begin();
      it != this->getCurrentLocation().indexs.end(); it++)
  {
    std::string resultPath = path + '/' + *it;
    if (this->isFile_(resultPath))
      return(true);
  }
  return (false);
}

void  Server::POSTMethod_(const int fd)
{
  LocationConfig  location = this->getCurrentLocation();
  if (location.upload_dir.empty())
  {
    this->respondMissingUploadDir(fd);
    return ;
  }
  else if (!this->directoryExists_(location.upload_dir))
  {
    this->respondWithUploadError(fd);
    return ;
  }
  else if (!this->_clients[fd]->getRequest().isBodySizeAllowed())
  {
    this->respondPayloadTooLarge(fd);
    return ;
  }
  std::string     localPath;
  std::string     extractUri;

  localPath = location.upload_dir + '/' + getUriPath_(fd).substr(location.path.size());
  this->_localPath = localPath;
  if (directoryExists_(localPath))
    this->saveUploadedFile_(fd);
  else
    this->respondMissingUploadDir(fd);
  logger(LOG_DEBUG, "value of path [" + localPath + "]");
}

bool  Server::isBodySizeAllowed(const int& fd)
{
  LocationConfig  location = this->getCurrentLocation();
  size_t          contentLength = this->_clients[fd]->getRequest().getBody().size();

  if (contentLength > location.client_max_body_size)
  {
    logger(LOG_DEBUG, "Too large");
    logger(LOG_INFO, 
      "❌ Client body size (" + toString(contentLength) + 
      " bytes) exeeds limit (" + 
      toString(location.client_max_body_size) + " bytes).");

    return (false);
  }
  return (true);
}

void  Server::saveMultipartFiles(const int& fd)
{
  std::vector<MultipartPart>& multipart = this->_clients[fd]->getRequest().getMultipart();
  LocationConfig location = this->_clients[fd]->getRequest().getLocation(); 
  for (std::vector<MultipartPart>::iterator it = multipart.begin(); it != multipart.end(); it++)
  {
    if (!it->fullySaved)
    {
      std::string localPath;
      if (it->filename.empty())
        localPath = location.upload_dir + "/" + unique_filename("upload.bin");
      else
        localPath = location.upload_dir + "/" + it->filename;
      std::ofstream file(localPath.c_str(), std::ios::app);
      if (!file.is_open())
      {
        logger(LOG_WARNING,"something wrong: it seems like file previously opened is not accessible"
            + localPath);
        return;
      }

      if (it->offset + READ_CHUNK_SIZE > it->data.size())
      {
        file.write(it->data.c_str() + it->offset, it->data.size() - it->offset);
        it->offset += it->data.size() - it->offset;
      }
      else
      {
        file.write(it->data.c_str() + it->offset, READ_CHUNK_SIZE);
        it->offset += READ_CHUNK_SIZE;
      }
      file.close();
      if (it->offset == it->data.size())
      {
        it->fullySaved = true;
        if (++it == multipart.end())
        {
          this->_clients[fd]->getRequest()._allFilesSaved = true;
          this->saveUploadedFile_(fd);
        }
      }
      return ;
    }
  }
}

void Server::saveBodyToBinary(const int& fd)
{
  std::string path;
  std::string filename;
  path = this->getCurrentLocation().upload_dir + "/" + unique_filename("upload.bin");
  std::ofstream out(path.c_str(), std::ios::out | std::ios::binary);
  if (!out)
  {
      std::cerr << "Unable to create the file : " << filename << std::endl;
      return;
  }
  const std::string& body = this->_clients[fd]->getRequest().getBody();
  out.write(body.c_str(), body.size());
  out.close();
  this->_clients[fd]->getRequest()._allFilesSaved = true;
}

void  Server::saveUploadedFile_(const int fd)
{
  logger(LOG_DEBUG, "In function saveUploadedFile_");
  std::string body;

  this->setStatus(201, fd);
  body = "File uploaded successfully.";

  std::string contentLength = toString(body.size());
  std::ostringstream headers;

  headers << this->getVersion(fd) << " 201 Created\r\n"
    << CT << " " << CT_TEXT << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}

void  Server::respondMissingUploadDir(const int fd)
{
  logger(LOG_DEBUG, "In function respondMissingUploadDir");
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(500, fd);
  if (this->hasCustomErrorPage(500, fd))
    this->saveErrorBodyFilePath(500, fd, contentType, contentLength);
  else
  {
    body = "Server misconfiguration: upload_dir not specified.";
    contentLength = toString(body.size());
  }

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 500 Internal Server Error\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}


void  Server::DELETEMethod_(const int fd)
{
  LocationConfig  location = this->getCurrentLocation();
  std::string     localPath;
  std::string     extractUri;
  if (this->getCurrentLocation().root.empty())
  {
    this->respondNotFound_(fd);
    return;
  }
  localPath = location.root + '/' + getUriPath_(fd).substr(location.path.size());
  this->_localPath = localPath;
  logger(LOG_DEBUG, "value of path [" + localPath + "]");
  if (isFile_(localPath))
  {
    if (remove(localPath.c_str()) == 0)
    {
      logger(LOG_INFO, "successfully detele file");
      this->onDeleteSuccess_(fd);
    }
    else
    {
      logger(LOG_INFO, "cannot delete file");
      this->cannotDeleteFile_(fd, localPath);
    }
  }
  else
  {
    logger(LOG_DEBUG, "Cannot delete the resource due to a conflict on the server.");
    this->respondNotFound_(fd);
  }
}

bool Server::directoryExists_(const std::string& path)
{
    struct stat st;
    return (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode));
}

void  Server::cannotDeleteFile_(const int fd, std::string& path)
{
  logger(LOG_DEBUG, "In function cannotDeleteFile_");
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(403, fd);
  if (this->hasCustomErrorPage(403, fd))
    this->saveErrorBodyFilePath(403, fd, contentType, contentLength);
  else
  {
    body = "You don't have permission to access '" + path + "'on this server.";
    contentLength = toString(body.size());
  }

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 409 Conflict\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}

void  Server::respondNotFound_(const int fd)
{
  logger(LOG_DEBUG, "In function respondNotFound_");
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(404, fd);
  if (this->hasCustomErrorPage(404, fd))
    this->saveErrorBodyFilePath(404, fd, contentType, contentLength);
  else
  {
    body = "404 Not Found: Invalid path.";
    contentLength = toString(body.size());
  }

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 404 Not Found\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}


void  Server::respondDirectoryListingForbidden(const int fd)
{
  logger(LOG_DEBUG, "In function respondDirectoryListingForbidden");
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(403, fd);
  if (this->hasCustomErrorPage(403, fd))
    this->saveErrorBodyFilePath(403, fd, contentType, contentLength);
  else
  {
    body = "403 Forbidden: Directory listing denied.";
    contentLength = toString(body.size());
  }

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 403 Forbidden\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}

void  Server::respondFileNotReadable(const int fd)
{
  logger(LOG_DEBUG, "In function respondFileNotReadable");
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(403, fd);
  if (this->hasCustomErrorPage(403, fd))
    this->saveErrorBodyFilePath(403, fd, contentType, contentLength);
  else
  {
    body = "You do not have permission to read the requested file.";
    contentLength = toString(body.size());
  }

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 403 Forbidden\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}

void  Server::buildDirectoryListing_(const int fd)
{
  const std::string body = this->generateAutoIndexHtml(fd);
  std::string contentLength = toString(body.size());
  std::string contentType = CT_HTML;
  this->setStatus(200, fd);

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 200 OK\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
  logger(LOG_INFO, "Autoindex response created for directory: " + this->getUriPath_(fd));
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}

const std::string Server::generateAutoIndexHtml(const int& fd)
{
  std::string uriPath = this->getUriPath_(fd);
  if (uriPath.at(uriPath.size() - 1) == '/')
    uriPath.erase(uriPath.size() - 1);

  const std::string localPath = this->getCurrentLocation().root
    + "/" + uriPath.substr(this->getCurrentLocation().path.size());
  DIR *dir_ptr = opendir(localPath.c_str());
  
  struct dirent *entry;

  std::string body;
  body = "<html>\n<head><title>Index of " + uriPath + "</title></head>\n<body>\n<h1>Index of "
    + uriPath + "</h1>\n<ul>\n";
  while ((entry = readdir(dir_ptr)) != NULL)
  {
    logger(LOG_INFO, "tonga eto ve");
    const std::string li = "<li><a href=" + uriPath + "/" + entry->d_name
      + ">" + entry->d_name + "</a></li>";
    body.append(li);
  }
  body.append("</ul>\n</body>\n</html>");
  closedir(dir_ptr);
  return (body);
}

void  Server::processReadableFile_(const int fd, const std::string& path)
{
  logger(LOG_DEBUG, "In function processReadableFile_");
  std::string contentType = this->getContentTypeByFileExtension(path);

  std::string contentLength = toString(getFileSize(path));
  this->setBodySize(fd, getFileSize(path));
  std::ostringstream headers;

  headers << this->getVersion(fd) << " 200 OK\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());

  this->setStatus(200, fd);
  this->setBodyFilePath(fd, path);
  this->_clients[fd]->getResponse().openAndSaveBodyFileStream(path);
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}

void  Server::onDeleteSuccess_(const int fd)
{
  logger(LOG_DEBUG, "In function onDeleteSuccess_");

  this->setStatus(204, fd);

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 204 No Content\r\n"
    << CL << " " << "0" << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody("");
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}

void  Server::methodNotAllowed_(const int fd)
{
  logger(LOG_DEBUG, "In function methodNotAllowed_");
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(405, fd);
  if (this->hasCustomErrorPage(405, fd))
    this->saveErrorBodyFilePath(405, fd, contentType, contentLength);
  else
  {
    body = "The method " + this->getMethod(fd) + " is not allowed on " + this->getCurrentLocation().path;
    contentLength = toString(body.size());
  }

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 405 Method Not Allowed\r\n"
    << "Allow: " << this->getAllowedMethodsForLocation() << "\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}


std::string Server::getAllowedMethodsForLocation(void)
{
  std::string allowedMethod;
  int i = 0;
  for (std::vector<std::string>::const_iterator it = this->getCurrentLocation().methods.begin();
      it != this->getCurrentLocation().methods.end(); it++)
  {
    if (i == 0)
    {
      allowedMethod = *it;
      i = 1;
    }
    else
      allowedMethod += ", " + *it;
  }
  return (allowedMethod);
}


void  Server::badRequest_(const int fd)
{
  logger(LOG_DEBUG, "In function badRequest_");
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(400, fd);
  if (this->hasCustomErrorPage(400, fd))
    this->saveErrorBodyFilePath(400, fd, contentType, contentLength);
  else
  {
    body = "400 Bad Request: Invalid HTTP request syntax.";
    contentLength = toString(body.size());
  }

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 400 Bad Request\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}

void  Server::handleNoMatchingLocation_(const int fd)
{

  logger(LOG_DEBUG, "In function handleNoMatchingLocation_");
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(404, fd);
  if (this->hasCustomErrorPage(404, fd))
    this->saveErrorBodyFilePath(404, fd, contentType, contentLength);
  else
  {
    body = "404 Not Found: The requested resource does not exist.";
    contentLength = toString(body.size());
  }

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 404 Not Found\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}

void Server::saveErrorBodyFilePath(const int code, const int& fd,
    std::string& contentType, std::string& contentLength)
{
  if (this->getCurrentLocation().path.empty())
  {
    ServerConfigConstIterator serverConf = this->_clients[fd]->getServerConfig();
    std::map<int, std::string> errorPages = serverConf->error_pages;
    for (std::map<int, std::string>::iterator it = errorPages.begin();
        it != errorPages.end(); it++)
    {
      if (it->first == code)
      {
        std::string path = serverConf->root + '/' + it->second;
        contentType = this->getContentTypeByFileExtension(path);
        contentLength = toString(getFileSize(path));
        this->_clients[fd]->getResponse().openAndSaveBodyFileStream(path);
        this->setBodySize(fd, getFileSize(path));
        this->setBodyFilePath(fd, path);
        return ;
      }
    }
  }
  else
  {
    std::map<int, std::string> errorPages = this->getCurrentLocation().error_pages;
    for (std::map<int, std::string>::iterator it = errorPages.begin();
        it != errorPages.end(); it++)
    {
      if (it->first == code)
      {
        std::string path = this->getCurrentLocation().root + '/' + it->second;
        contentType = this->getContentTypeByFileExtension(path);
        contentLength = toString(getFileSize(path));
        this->_clients[fd]->getResponse().openAndSaveBodyFileStream(path);
        this->setBodySize(fd, getFileSize(path));
        this->setBodyFilePath(fd, path);
        return ;
      }
    }
  }
  logger(LOG_ERROR, "Error detected, No body file path found");
  contentType = CT_TEXT;
  contentLength = "0";
}

std::string Server::readLocalFileToString(std::string path)
{
  std::ifstream file(path.c_str(), std::ios::binary);
  if (file.is_open())
  {
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return (buffer.str());
  }
  logger(LOG_ERROR, "⚠️ Could not open file: " + path);
  return ("");
}

std::string Server::buildConnectionHeader(const int fd)
{
  if (this->getVersion(fd) == "HTTP/1.0"
      || !this->_clients[fd]->getRequest().isBodySizeAllowed())
  {
    this->_clients[fd]->getResponse().setKeepAliveStatus(false);
    return "Connection: close\r\n\r\n";
  }

  const std::map<std::string, std::string>& headers = this->getHeaders(fd);
  for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
  {
    if (caseInsensitiveEqual(it->first, "connection"))
    {
      if (caseInsensitiveEqual(it->second, "close"))
      {
        this->_clients[fd]->getResponse().setKeepAliveStatus(false);
        return "Connection: close\r\n\r\n";
      }
      else
      {
        this->_clients[fd]->getResponse().setKeepAliveStatus(true);
        return "Connection: keep-alive\r\n\r\n";
      }
    }
  }
  this->_clients[fd]->getResponse().setKeepAliveStatus(true);
  return "Connection: keep-alive\r\n\r\n";
}

bool  Server::hasCustomErrorPage(const int code, const int fd)
{
  if (this->getCurrentLocation().path.empty())
  {
    ServerConfigConstIterator serverConf = this->_clients[fd]->getServerConfig();
    std::map<int, std::string> errorPages = serverConf->error_pages;

    for (std::map<int, std::string>::iterator it = errorPages.begin(); it != errorPages.end(); it++)
    {
      if (it->first == code)
      {
        std::string path = serverConf->root + '/' + it->second;
        std::ifstream file(path.c_str());
        if (file.is_open())
        {
          file.close();
          logger(LOG_INFO, toString(code) + " page found and readable: [" + toString(code) + "]");
          return (true);
        }
        break ;
      }
    }
  }
  else
  {
    std::map<int, std::string> errorPages = this->getCurrentLocation().error_pages;

    for (std::map<int, std::string>::iterator it = errorPages.begin(); it != errorPages.end(); it++)
    {
      if (it->first == code)
      {
        std::string path = this->getCurrentLocation().root + '/' + it->second;
        std::ifstream file(path.c_str());
        if (file.is_open())
        {
          file.close();
          logger(LOG_INFO, toString(code) + " page found and readable: [" + toString(code) + "]");
          return (true);
        }
        break ;
      }
    }
  }
  return (false);
}

void Server::loadMimeTypes(void)
{
  std::ifstream file("./config/mimes.types");
  if (!file.is_open())
    throwWithLog(LOG_ERROR, "Failed to load MIME types: file 'config/mimes.types' not found.");

  std::string line;
  std::string header;

  std::getline(file, header);
  if (header != "# mime.types - basic MIME type mappings")
  {
    file.close();
    return ;
  }
  while (std::getline(file, line))
  {
    std::istringstream  iss(line);
    std::string         key;
    std::string         value;

    iss >> value >> key;
    if (value.empty())
      continue ;
    if (key.empty())
    {
      file.close();
      throwWithLog(LOG_ERROR, "invalid file format[" + line + "]");
    }
    this->_mimes[key]  = value;
    while (1)
    {
      key = "";
      iss >> key;
      if  (key.empty())
        break ;
      this->_mimes[key]  = value;
    }
  }
  logger(LOG_INFO, "mimes.type has been correctly load");
  file.close();
}

std::string Server::getContentTypeByFileExtension(std::string path)
{
  std::string fileName;
  size_t      slashPos;

  slashPos = path.rfind('/');
  if (slashPos == std::string::npos)
  {
    fileName = path;
    logger(LOG_WARNING, "Error detected, the path need to have at least one '/'");
  }
  else
    fileName = path.substr(slashPos + 1);

  size_t extStart = path.rfind('.');
  if (extStart == std::string::npos)
    return (CT_TEXT);
  else
  {
    std::string::iterator it = path.begin() + extStart + 1;
    if (it == path.end())
      return (CT_TEXT);

    std::string ext = path.substr(extStart + 1);
    for (std::map<std::string, std::string>::iterator it = this->_mimes.begin();
        it != this->_mimes.end(); ++it)
    {
      if (it->first == ext)
        return (it->second);
    }
  }
  return (CT_TEXT);
}

void  Server::handleRedirect_(const int& fd)
{
  if (this->getCurrentLocation().redirect.size() > 1)
    logger(LOG_WARNING, "Parsing error: invalid or unexpected configuration format.");
  std::map<int, std::string>::const_iterator it = this->getCurrentLocation().redirect.begin();
  if (this->isRedirectCode_(it->first))
  {
    if (it->second.empty())
      this->handleReturnWithoutUrl_(fd);
    else
      this->respondRedirect_(fd, it);
  }
  else if (this->isValidHttpStatusCode_(it->first))
    this->respondNotImplemented_(fd);
  else
    this->respondInternalServerError(fd);
}

void  Server::respondNotImplemented_(const int& fd)
{
  logger(LOG_DEBUG, "In function respondNotImplemented_");
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(501, fd);
  if (this->hasCustomErrorPage(501, fd))
    this->saveErrorBodyFilePath(501, fd, contentType, contentLength);
  else
  {
    body = "501 Not Implemented";
    contentLength = toString(body.size());
  }

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 501 Not Implemented\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}

bool  Server::isRedirectCode_(int statusCode)
{
    return ((statusCode >= 301 && statusCode <= 303)
        || statusCode == 307 || statusCode == 308);
}

void  Server::respondRedirect_(const int& fd,
    const std::map<int, std::string>::const_iterator it)
{
  logger(LOG_DEBUG, "In function respondRedirect_");
  std::ostringstream  headers;
  std::string         status_text;
  std::string         body;

  switch (it->first)
  {
    case 301:
      status_text = " Moved Permanently";
      body = "301 Moved Permanently";
      break;
    case 302:
      status_text = " Found";
      body = "302 Found";
      break;
    case 303:
      status_text = " See Other";
      body = "303 See Other";
      break;
    case 307:
      status_text = " Temporary Redirect";
      body = "307 Temporary Redirect";
      break;
    case 308:
      status_text = " Permanent Redirect";
      body = "308 Permanent Redirect";
      break;
    default:
      status_text = "";
      break;
  }
  headers << this->getVersion(fd) << " " << it->first << status_text << "\r\n"
    << "Location: " << it->second << "\r\n"
    << CL << " " << body.size() << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
  this->setStatus(it->first, fd);
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}

void  Server::handleReturnWithoutUrl_(const int& fd)
{
  logger(LOG_DEBUG, "In function handleReturnWithoutUrl");
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(403, fd);
  if (this->hasCustomErrorPage(403, fd))
    this->saveErrorBodyFilePath(403, fd, contentType, contentLength);
  else
  {
    body = "Access denied: Forbidden.";
    contentLength = toString(body.size());
  }

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 403 Forbidden\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}

bool  Server::isValidHttpStatusCode_(const int& code)
{
  return (code >= 100 && code <= 599);
}

void  Server::setBodyFilePath(const int& fd, const std::string& path)
{
  this->_clients[fd]->getResponse().setBodyFilePath(path);
}

void  Server::setBodySize(const int& fd, const ssize_t& bodySize)
{
  this->_clients[fd]->getResponse().setBodySize(bodySize);
}

void  Server::respondPayloadTooLarge(const int& fd)
{
  logger(LOG_DEBUG, "In function respondPayloadTooLarge");
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(413, fd);
  if (this->hasCustomErrorPage(413, fd))
    this->saveErrorBodyFilePath(413, fd, contentType, contentLength);
  else
  {
    body = "413 Request Entity Too Large";
    contentLength = toString(body.size());
  }

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 413 Request Entity Too Large\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
  this->saveHeaderAndBodySize(fd);
  this->setPollOut_(fd);
}

// TODO 
void  Server::respondFallbackError(const int& fd)
{
  logger(LOG_DEBUG, "In function respondFallbackError");
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(502, fd);
  if (this->hasCustomErrorPage(502, fd))
    this->saveErrorBodyFilePath(502, fd, contentType, contentLength);
  else
  {
    body = "Failed to retrieve response from CGI.";
    contentLength = toString(body.size());
  }

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 502 Bad Gateway\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
}
