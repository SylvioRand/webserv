/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 10:26:47 by srandria          #+#    #+#             */
/*   Updated: 2025/07/31 09:52:28 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/Server.hpp"
#include <algorithm>
#include <csignal>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

Server::Server(const Config& config) : _config(config)
{
  this->start_server_();
}

Server::~Server(void)
{

}

void  Server::start_server_(void)
{
  logger(LOG_INFO, "Server is starting");
  this->create_all_listeners_();
  while (true) { int ready = poll(&_pool_fds[0], _pool_fds.size(), -1);
    if (ready == -1)
      throwWithLog(LOG_FATAL, "poll() failed");

    for (size_t i = 0; i < _pool_fds.size() && ready > 0; ++i)
    {
      if (_pool_fds[i].revents == 0)
        continue;
      --ready;

      int fd = _pool_fds[i].fd;

      if (std::find(_listener_fds.begin(), _listener_fds.end(), fd) != _listener_fds.end()
          && (_pool_fds[i].revents & POLLIN))
      {
        this->accept_new_client_(fd);
      }
      else if ((this->_clients).find(fd) != _clients.end())
      {
        if (_pool_fds[i].revents & POLLIN)
          this->handle_pollin_(fd);
        if (_pool_fds[i].revents & POLLOUT)
          this->handle_pollout_(fd);
        if (_pool_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
          this->close_client_(fd);
      }
    }
    check_timout_();
  }
}

// TODO
void  Server::stop_server(void)
{

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

    this->buildIpv4Sockaddr_(addr, cfg);       // fill struct sockaddr_in for binding
    this->bindSocket_(fd, cfg, addr);
    this->startListener_(fd, cfg);
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


void Server::startListener_(int fd, const ServerConfig& cfg)
{
  std::ostringstream oss;

  if (listen(fd, 128))
  {
    close(fd);
    oss << "listen() failed : " << cfg.host << ":" << cfg.port;
    throwWithLog(LOG_FATAL, oss.str());
  }

  oss << "🟢 listening on http://" << cfg.host << ":" << cfg.port;
  logger(LOG_INFO, oss.str());

  _listener_fds.push_back(fd);
}

void  Server::addFdToPoll_(int fd)
{
  struct pollfd pfd;
  pfd.fd = fd;
  pfd.events = POLLIN;
  pfd.revents = 0;            // poll() will overwrite this value when an event is captured.

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
  _clients[client_fd] = new Client(client_fd);

  this->addFdToPoll_(client_fd);
}

void  Server::setNonBlocking_(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        throwWithLog(LOG_FATAL, "fcntl(F_GETFL) failed");
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        throwWithLog(LOG_FATAL, "fcntl(F_SETFL, O_NONBLOCK) failed");
}

// TODO
void  Server::handle_pollin_(int fd)
{
  std::ostringstream os;
  os << "HANDLE POLLIN FD ->" << fd;
  logger(LOG_DEBUG, os.str());
  Client *client = this->_clients[fd];
  (*client).readData();
  if ((*client).isRequestComplete())
  {
    std::string method = this->getMethod(fd);
    if (!this->isHttpMethodValid_(method))
    {
      this->badRequest_(fd);
      this->setPollOut_(fd);
      return ;
    }
    std::string uri = this->getUri_(fd);
    logger(LOG_INFO, "the path(uri) [" +  uri + "]");
    ServerConfigConstIterator serverConf = this->findMatchingServer(fd);
    logger(LOG_INFO, "the path(uri) [" +  uri + "]");
    saveMatchingLocation_(uri, serverConf);
    if (!isSupportedHttpMethod(method))
      this->methodNotSupported_(fd);
    else if (getCurrentLocation().methods.empty())
      this->methodNotAllowed_(fd);
    else if (method == "GET" && this->isMethodAllowedForLocation("GET"))
      this->GETMethod_(uri, fd);
    else if (method == "POST" && this->isMethodAllowedForLocation("POST"))
      this->POSTMethod_(uri, fd);
    else if (method == "DELETE" && this->isMethodAllowedForLocation("DELETE"))
      this->DELETEMethod_(uri, fd);
    else
      this->methodNotAllowed_(fd);
    logger(LOG_DEBUG, "value of status" + intToString(this->_clients[fd]->getResponse().getStatus()));
    this->setPollOut_(fd);
  }
}
const LocationConfig& Server::getCurrentLocation(void)
{
  return (this->_currentLocation);
}

std::string  Server::getMethod(int fd)
{
  return (this->_clients[fd]->getRequest().getMethod());
}

std::string Server::getUri_(int fd)
{
  return (this->_clients[fd]->getRequest().getPath());
}

void  Server::setStatus(int code, int fd)
{
  this->_clients[fd]->getResponse().setStatus(code);
}

bool  Server::isMethodAllowedForLocation(const std::string method)
{
  std::vector<std::string>::const_iterator it = std::find(this->getCurrentLocation().methods.begin(),
        this->getCurrentLocation().methods.end(), method);
  if (it != this->getCurrentLocation().methods.end())
  {
    std::cout << "method [" << method << "] is detected and allowed" << std::endl;
    return (true);
  }
  std::cout << "method [" << method << "] is detected but not allowed" << std::endl;
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

void  Server::methodNotSupported_(const int fd)
{

  /*
  HTTP/1.1 501 Not Implemented
  Content-Type: text/plain
  Content-Length: 56

  The HTTP method PUT is recognized but not supported by this server.
  */
  this->setStatus(501, fd);
}

void  Server::setPollOut_(int fd)
{
  logger(LOG_INFO, "HANDLE POLLOUT");
  for (std::vector<struct pollfd>::iterator it = _pool_fds.begin(); it != _pool_fds.end(); ++it)
  {
    if (it->fd == fd) { it->events = POLLOUT;
      std::ostringstream oss;

      oss << "successfully set event to POLL_OUT for fd [" << fd << "]";
      logger(LOG_DEBUG, oss.str());

      return ;
    }
  }
  logger(LOG_ERROR, "fd not found");
}

// TODO
void  Server::handle_pollout_(int fd)
{
//  logger(LOG_INFO, "IN POLLOUT FUNCTION");
  this->_clients[fd]->sendData();
}

// TODO
void  Server::close_client_(int fd)
{
  logger(LOG_INFO, "close client");
  close(fd);
}

const std::map<std::string, std::string>& Server::getHeaders(int fd)
{
    return (_clients[fd]->getRequest().getHeaders());
}

Server::ServerConfigConstIterator Server::findMatchingServer(int fd)
{
  const std::map<std::string, std::string>& headers = this->getHeaders(fd);
  std::map<std::string, std::string>::const_iterator hostHeader = headers.find("Host");

  if (hostHeader == headers.end())
    throwWithLog(LOG_ERROR, "The Client Request doesn't have Host in _headers");

  std::string hostFromRequest = hostHeader->second;

  const std::vector<ServerConfig>&  servers = this->getServers();
  for (ServerConfigConstIterator cfg = servers.begin(); cfg != servers.end(); ++cfg)
  {
    std::string hostPort = cfg->host + ":" + intToString((*cfg).port);
    if (hostPort == hostFromRequest)
    {
      logger(LOG_INFO, "Config found for corresponding host -> " + hostFromRequest);
      return (cfg);
    }
  }
  throwWithLog(LOG_ERROR, "No matching server configuration found for host: " + hostFromRequest);
  return (servers.end());
}

void Server::saveMatchingLocation_(const std::string& uri, ServerConfigConstIterator& cfg)
{
  LocationConfig  best_match;
  this->setCurrentLocation(best_match);
  size_t best_length = 0;

  std::map<std::string, LocationConfig> locations = cfg->locations;
  std::cout << "verify uri []" << uri << "]" << std::endl;
  for (std::map<std::string, LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); it++)
  {
    std::cout << "location root [" << it->second.root << "]" << std::endl;
    const std::string& path = it->first;
    std::cout << "location path [" << it->first << "]" << std::endl;
    std::cout << "sizeof(path) [" << sizeof(path) << "]" << std::endl;
    std::cout << "sub ["<< uri.substr(0, sizeof(path)) << "]" << std::endl;
    if (uri.substr(0, path.size()) == path && path.size() > best_length)
    {
      std::cout << "are iU here" << std::endl;
      best_match = it->second;
      best_length = path.size();
    }
  }
  if (best_length == 0)
  {
    throwWithLog(LOG_FATAL, "No matching LocationConfig found");
  }
  this->setCurrentLocation(best_match);
  logger(LOG_DEBUG, "matching LocationConfig found, PATH [" + best_match.root + "]");
}

// TODO
void  Server::check_timout_(void)
{

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

void  Server::GETMethod_(std::string& uri, const int fd)
{
  LocationConfig  location = this->getCurrentLocation();
  std::string     path;
  std::string     extractUri;
  path = location.root + '/' + uri.substr(location.path.size());
  logger(LOG_DEBUG, "value of path [" + path + "]");
  if (this->isFile_(path))
  {
    if (this->isReadable_(path))
      this->processReadableFile_(fd);
    else
      this->respondFileNotReadable(fd);
  }
  else if (!directoryExists_(path))
    this->respondNotFound_(fd);
  else if (hasIndexDirective_(path))
  {
    std::string indexPath = this->getAccessibleIndexPath_(path);
    if (indexPath.empty())
    {
      if (this->existsAtLeastOneIndexFile_(path, fd))
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

void  Server::respondIndexFilesUnreadable_(const int fd)
{
  /*
   HTTP/1.1 403 Forbidden
  Content-Type: text/plain
  Content-Length: 53

  403 Forbidden: No readable index file found in this directory.
  */
  this->setStatus(403, fd);
}

void  Server::respondNoIndexFileFound_(const int fd)
{
  /*
   HTTP/1.1 404 Not Found
  Content-Type: text/plain
  Content-Length: 43

  404 Not Found: No index file found in this directory.
  */
  this->setStatus(404, fd);
}


void  Server::serveIndexContent_(const std::string path, const int fd)
{
  /*
  HTTP/1.1 200 OK
  Content-Type: [type MIME]
  Content-Length: [taille du fichier]
  ...

  [corps du fichier]
  */
  this->setStatus(200, fd);
}

bool  Server::hasIndexDirective_(const std::string& path)
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
    std::string resultPath = path + *it;
    if (this->isFile_(resultPath) && this->isReadable_(resultPath))
      return(resultPath);
  }
  return ("");
}

bool  Server::existsAtLeastOneIndexFile_(const std::string path, const int fd)
{
  for (std::vector<std::string>::const_iterator it = this->getCurrentLocation().indexs.begin();
      it != this->getCurrentLocation().indexs.end(); it++)
  {
    std::string resultPath = path + *it;
    if (this->isFile_(resultPath))
      return(true);
  }
  return (false);
}


// TODO
void  Server::POSTMethod_(std::string& uri, const int fd)
{
  LocationConfig  location = this->getCurrentLocation();
  if (location.upload_dir.empty())
  {
    this->respondMissingUploadDir(fd);
    return ;
  }
  std::string     localPath;
  std::string     extractUri;
  localPath = location.upload_dir + '/' + uri.substr(location.path.size());
  if (directoryExists_(localPath))
  {
    // TODO on doit obligatoirement passer par poll meme si on upload un fichier
    // atooooooooooo
    this->saveUploadedFile_(fd);
  }
  logger(LOG_DEBUG, "value of path [" + localPath + "]");
}

void  Server::saveUploadedFile_(const int fd)
{
  /*
  HTTP/1.1 201 Created
  Content-Type: text/plain
  Content-Length: 29

  File uploaded successfully.
  */
  this->setStatus(201, fd);
}

void  Server::respondMissingUploadDir(const int fd)
{
  /*
   HTTP/1.1 500 Internal Server Error
  Content-Type: text/plain
  Content-Length: 49

  Server misconfiguration: upload_dir not specified.
  */
  this->setStatus(500, fd);
}


/// TODO
void  Server::DELETEMethod_(std::string& uri, const int fd)
{
  std::cout << "On delete method" << std::endl;
  LocationConfig  location = this->getCurrentLocation();
  std::string     localPath;
  std::string     extractUri;
  localPath = location.root + '/' + uri.substr(location.path.size());
  logger(LOG_DEBUG, "value of path [" + localPath + "]");
  if (isFile_(localPath))
  {
    if (remove(localPath.c_str()) == 0)
    {
      logger(LOG_DEBUG, "successfully detele file");
      this->onDeleteSuccess_(fd);
    }
    else
    {
      logger(LOG_DEBUG, "cannot delete file");
      this->cannotDeleteFile_(fd);
    }
  }
  else
  {
    logger(LOG_DEBUG, "Cannot delete the resource due to a conflict on the server.");
    this->respondDeleteDirConflict_(fd);
  }
}

bool Server::directoryExists_(const std::string& path)
{
    struct stat st;
    return (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode));
}

// TODO
void  Server::cannotDeleteFile_(const int fd)
{
  /*
  HTTP/1.1 409 Conflict
  Content-Type: text/plain
  Content-Length: 65

  Conflict: Unable to delete '/path/to/file' due to current resource state
  */
  this->setStatus(409, fd);
}

// TODO
void  Server::respondNotFound_(const int fd)
{
  /*
  HTTP/1.1 404 Not Found
  Content-Type: text/plain
  Content-Length: 23

  404 Not Found: Invalid path.
  */
  this->setStatus(404, fd);
}


// TODO
void  Server::respondDirectoryListingForbidden(const int fd)
{
  /*
  HTTP/1.1 403 Forbidden
  Content-Type: text/plain
  Content-Length: 35

  403 Forbidden: Directory listing denied.
  */
  this->setStatus(403, fd);
}

// TODO
void  Server::respondFileNotReadable(const int fd)
{
  /*
  HTTP/1.1 403 Forbidden
  Content-Type: text/plain
  Content-Length: 48

  You do not have permission to read the requested file.
  */
  this->setStatus(403, fd);
}

// TODO
void  Server::respondDeleteDirConflict_(const int fd)
{
  /*
  HTTP/1.1 409 Conflict
  Content-Type: text/plain
  Content-Length: XX

  Cannot delete resource due to a conflict with the current state.
  */
  this->setStatus(409, fd);
}

// TODO
void  Server::buildDirectoryListing_(const int fd)
{
  /*
  HTTP/1.1 200 OK
  Content-Type: text/html
  Content-Length: [longueur]

  <html>
  <head><title>Index of /images</title></head>
  <body>
  <h1>Index of /images</h1>
  <ul>
    <li><a href="photo1.jpg">photo1.jpg</a></li>
    <li><a href="photo2.jpg">photo2.jpg</a></li>
  </ul>
  </body>
  </html>
  */
  this->setStatus(200, fd);
}

// TODO
void  Server::processReadableFile_(const int fd)
{
  this->setStatus(200, fd);
}

// TODO
void  Server::onDeleteSuccess_(const int fd)
{
  /*
  HTTP/1.1 204 No Content
  Content-Length: 0
  */
  this->setStatus(200, fd);
}

// TODO
void  Server::methodNotAllowed_(const int fd)
{
  /*
  HTTP/1.1 405 Method Not Allowed
  Allow: 
  Content-Type: text/plain
  Content-Length: 46

  The method GET is not allowed on /restricted.
  */
  this->setStatus(405, fd);
}

// TODO
void  Server::badRequest_(const int fd)
{
  /*
  HTTP/1.1 400 Bad Request
  Content-Type: text/plain
  Content-Length: 42

  400 Bad Request: Invalid HTTP request syntax.
  */
  this->setStatus(405, fd);
}
