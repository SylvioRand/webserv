/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 10:26:47 by srandria          #+#    #+#             */
/*   Updated: 2025/08/15 09:58:56 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/Server.hpp"
#include <algorithm>
#include <cctype>
#include <csignal>
#include <fcntl.h>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

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
  this->create_all_listeners_();

  while (true)
  {
    logger(LOG_DEBUG, "Waiting on poll...");
    int ready = poll(&_pool_fds[0], _pool_fds.size(), -1);
    if (ready == -1)
      throwWithLog(LOG_FATAL, "poll() failed");

    // On itère à l'envers pour éviter les problèmes avec erase()
    for (int i = static_cast<int>(_pool_fds.size()) - 1; i >= 0 && ready > 0; --i)
    {
      if (_pool_fds[i].revents == 0)
        continue;

      --ready;
      int fd = _pool_fds[i].fd;
      short revents = _pool_fds[i].revents;

      // Si c'est un listener
      if (std::find(_listener_fds.begin(), _listener_fds.end(), fd) != _listener_fds.end())
      {
        if (revents & POLLIN)
          this->accept_new_client_(fd);
      }
      // Sinon, c'est un client connu
      else if (_clients.find(fd) != _clients.end())
      {
        if (revents & POLLIN)
          this->handle_pollin_(fd);
        if (revents & POLLOUT)
          this->handle_pollout_(fd);
        if (revents & (POLLERR | POLLHUP | POLLNVAL))
          this->close_client_(fd);
      }
      else
        this->close_client_(fd);
    }
    logger(LOG_DEBUG, "Checking timeouts...");
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
  _clients[client_fd] = new Client(client_fd, this->_serverListeners[listener_fd]);
  this->addFdToPoll_(client_fd);
}

void  Server::setNonBlocking_(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0); if (flags == -1)
        throwWithLog(LOG_FATAL, "fcntl(F_GETFL) failed");
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        throwWithLog(LOG_FATAL, "fcntl(F_SETFL, O_NONBLOCK) failed");
}

void  Server::handle_pollin_(int fd)
{
  std::ostringstream os;
  os << "HANDLE POLLIN FD -> " << fd;
  logger(LOG_DEBUG, os.str());
  //this->_clients[fd]->getRequest().shiftBufferAfterRequest();
  Client *client = this->_clients[fd];
  if (!(*client).readData())
  {
    logger(LOG_DEBUG, "Cant` read data with readData()");
    close_client_(fd);
    return ;
  }
  if ((*client).isRequestComplete())
  {
    this->_clients[fd]->getRequest().setServerConf(this->_clients[fd]->getServerConfig());
    logger(LOG_DEBUG, "request completed");
    std::string method = this->getMethod(fd);
    if (!this->isHttpMethodValid_(method))
    {
      this->badRequest_(fd);
      this->saveHeaderAndBodySize(fd);
      this->setPollOut_(fd);
      return ;
    }
    std::string uri = this->getUri_(fd);
    logger(LOG_INFO, "the path(uri) [" +  uri + "]");
    ServerConfigConstIterator serverConf = this->_clients[fd]->getServerConfig();
    logger(LOG_INFO, "the path(uri) [" +  uri + "]");
    saveMatchingLocation_(uri, serverConf);
    for (std::vector<std::string>::const_iterator val = this->getCurrentLocation().methods.begin();
      val != this->getCurrentLocation().methods.end(); val++)
    {
      std::cout << "val = " << val->c_str() << std::endl;
    }

    if (this->getCurrentLocation().path.empty())
      this->handleNoMatchingLocation_(fd);
    else if (!this->getCurrentLocation().redirect.empty())
      this->handleRedirect_(fd);
    else if (!isSupportedHttpMethod(method))
      this->methodNotSupported_(fd);
    else if (getCurrentLocation().methods.empty())
      this->methodNotAllowed_(fd);
    else if (method == "GET" && this->isMethodAllowedForLocation("GET"))
      this->GETMethod_(uri, fd);
    else if (method == "POST" && this->isMethodAllowedForLocation("POST"))
      this->POSTMethod_(uri, fd);
    else if (method == "DELETE" && this->isMethodAllowedForLocation("DELETE"))
      this->DELETEMethod_(uri, fd);
    else if (this->getMethod(fd) == "POST" && !this->_clients[fd]->getRequest().isBodySizeAllowed())
      this->respondPayloadTooLarge(fd);
    else
      this->methodNotAllowed_(fd);
    this->saveHeaderAndBodySize(fd);
    this->setPollOut_(fd);
  }
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


std::string Server::getUri_(int fd)
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
  for (std::vector<std::string>::const_iterator val = this->getCurrentLocation().methods.begin();
      val != this->getCurrentLocation().methods.end(); val++)
  {
    std::cout << "val = " << val->c_str() << std::endl;
  }
  std::vector<std::string>::const_iterator it = std::find(this->getCurrentLocation().methods.begin(),
        this->getCurrentLocation().methods.end(), method);
  if (it != this->getCurrentLocation().methods.end())
  {
    logger(LOG_DEBUG, "method [" + method + "] is detected and allowed");
    return (true);
  }
  logger(LOG_DEBUG, "method [" + method + "] is detected but not allowed");
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
  logger(LOG_DEBUG, "In function methodNotSupported_");
  /*
  HTTP/1.1 501 Not Implemented
  Content-Type: text/plain
  Content-Length: 56

  The HTTP method PUT is recognized but not supported by this server.
  */
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(501, fd);
  if (this->hasCustomErrorPage(501, fd))
    this->saveErrorBodyFilePath(501, fd, contentType, contentLength);
  else
  {
    body = "The HTTP method PUT is recognized but not supported by this server.";
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
}

void  Server::setPollOut_(int fd)
{
  logger(LOG_INFO, "HANDLE POLLOUT");
  for (std::vector<struct pollfd>::iterator it = _pool_fds.begin(); it != _pool_fds.end(); ++it)
  {
    if (it->fd == fd)
    {
      it->events = POLLOUT;
      std::ostringstream oss;

      oss << "successfully set event to POLL_OUT for fd [" << fd << "]";
      logger(LOG_DEBUG, oss.str());

      return ;
    }
  }
  logger(LOG_ERROR, "fd not found");
}

void  Server::setPollIn_(const int& fd)
{
  logger(LOG_INFO, "HANDLE POLL_IN");
  this->_clients[fd]->getRequest().shiftBufferAfterRequest();
  this->_clients[fd]->clearBuffer();
  for (std::vector<struct pollfd>::iterator it = _pool_fds.begin(); it != _pool_fds.end(); ++it)
  {
    if (it->fd == fd)
    {
      it->events = POLLIN;
      std::ostringstream oss;

      oss << "successfully set event to POLL_IN for fd [" << fd << "]";
      logger(LOG_DEBUG, oss.str());

      return ;
    }
  }
  logger(LOG_ERROR, "fd not found");
}


// TODO
void  Server::handle_pollout_(int fd)
{
  logger(LOG_INFO, "In handle_pollout_ FUNCTION");
  
  this->_clients[fd]->sendData(this->_localPath);
  logger(LOG_DEBUG, "in handle_pollout_ function");
  if (this->_clients[fd]->getResponse().areHeadersFullySent() &&
      this->_clients[fd]->getResponse().isBodyFullySent())
  {
    std::cout << "tokony efa vita eh" << std::endl;
    if (this->_clients[fd]->getResponse().isKeepAlive())
    {
      this->setPollIn_(fd);
      this->_clients[fd]->getResponse().initializeState();
    }
    else
      close_client_(fd);
  }
}

void Server::close_client_(int fd)
{
  logger(LOG_INFO, "close client");

  std::map<int, Client*>::iterator it = _clients.find(fd);
  if (it != _clients.end())
  {
    delete it->second;
    _clients.erase(it);
  }
  std::cout << "after removing client in _clients maps" << std::endl;
  close(fd);
  std::cout << "after close(fd)" << std::endl;
  std::vector<struct pollfd>::iterator itPollFd = this->_pool_fds.begin();
  while (itPollFd != this->_pool_fds.end())
  {
    if (itPollFd->fd == fd)
    {
      itPollFd = this->_pool_fds.erase(itPollFd);
      std::cout << "after removing client in poll" << std::endl;
      break;
    }
    else
      ++itPollFd;
  }
  logger(LOG_DEBUG, "Out");
}

const std::map<std::string, std::string>& Server::getHeaders(int fd)
{
    return (_clients[fd]->getRequest().getHeaders());
}

void Server::saveMatchingLocation_(const std::string& uri, ServerConfigConstIterator& cfg)
{
  LocationConfig  best_match;
  this->setCurrentLocation(best_match);
  size_t best_length = 0;

  std::map<std::string, LocationConfig> locations = cfg->locations;
  std::cout << "verify uri [" << uri << "]" << std::endl;
  for (std::map<std::string, LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); it++)
  {
    std::cout << "location root [" << it->second.root << "]" << std::endl;
    const std::string& path = it->first;
    std::cout << "location path [" << it->first << "]" << std::endl;
    if (uri.substr(0, path.size()) == path && path.size() > best_length)
    {
      best_match = it->second;
      best_length = path.size();
    }
  }
  if (best_length == 0)
  {
    logger(LOG_DEBUG, "root location will ve created and used");
    this->createAndSaveRootLocation_(cfg);
    return ;
  }
  this->setCurrentLocation(best_match);
  logger(LOG_DEBUG, "matching LocationConfig found, root [" + best_match.root + "]");
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
  this->_localPath = path;
  logger(LOG_DEBUG, "value of path [" + path + "]");
  if (this->getFileExtension_(path) == this->getCurrentLocation().cgi_extension
      && !this->getCurrentLocation().cgi_extension.empty()
      && !this->getCurrentLocation().cgi_path.empty())
    this->handleCgiGetRequest_(path, fd);
  else if (this->isFile_(path))
  {
    if (this->isReadable_(path))
      this->processReadableFile_(fd, path);
    else
      this->respondFileNotReadable(fd);
  }
  else if (!directoryExists_(path))
    this->respondNotFound_(fd);
  else if (hasIndexDirective_())
  {
    std::string indexPath = this->getAccessibleIndexPath_(path);
    if (indexPath.empty())
    {
      if (this->getCurrentLocation().autoindex)
        this->buildDirectoryListing_(fd);
      else if (this->existsAtLeastOneIndexFile_(path))
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

std::string Server::getFileExtension_(std::string& path)
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
    return ("");
  else
    return (path.substr(extStart));
  return ("");
}

bool Server::is_executable_file_(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        // Vérifie si c'est un fichier régulier
        if (S_ISREG(st.st_mode)) {
            // Vérifie s'il est exécutable par quelqu’un (propriétaire, groupe ou autres)
            if (st.st_mode & S_IXUSR || st.st_mode & S_IXGRP || st.st_mode & S_IXOTH) {
                return true;
            }
        }
    }
    return false;
}


void  Server::respondIndexFilesUnreadable_(const int fd)
{
  logger(LOG_DEBUG, "In function respondIndexFilesUnreadable_");
  /*
   HTTP/1.1 403 Forbidden
  Content-Type: text/plain
  Content-Length: 53

  403 Forbidden: No readable index file found in this directory.
  */
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
}

void  Server::respondNoIndexFileFound_(const int fd)
{
  logger(LOG_DEBUG, "In function respondNoIndexFileFound_");
  /*
   HTTP/1.1 404 Not Found
  Content-Type: text/plain
  Content-Length: 43

  404 Not Found: No index file found in this directory.
  */
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(404, fd);
  if (this->hasCustomErrorPage(404, fd))
    this->saveErrorBodyFilePath(404, fd, contentType, contentLength);
  else
  {
    body = "404 Not Found: No index file found in this directory.";
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
}

void  Server::serveIndexContent_(const std::string path, const int fd)
{
  logger(LOG_DEBUG, "In function serveIndexContent_");
  /*
  HTTP/1.1 200 OK
  Content-Type: [type MIME]
  Content-Length: [taille du fichier]
  ...

  [corps du fichier]
  */
  std::string contentType = this->getContentTypeByFileExtension(path);
  std::cout << "value of Content-Type " << contentType << std::endl;

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
  this->openAndSaveBodyFileFd(path, fd);
  this->setBodySize(fd, getFileSize(path));
  this->setBodyFilePath(fd, path);
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
  this->_localPath = localPath;
  if( this->getFileExtension_(localPath) == this->getCurrentLocation().cgi_extension
      && !this->getCurrentLocation().cgi_extension.empty()
      && !this->getCurrentLocation().cgi_path.empty())
    this->handleCgiPostRequest_(localPath, fd);
  else if (directoryExists_(localPath))
  {
    // TODO maybe you need more else if
    // TODO Need to parse the body before saving correct data to save in specific file
    this->saveUploadedFile_(fd);
  }
  logger(LOG_DEBUG, "value of path [" + localPath + "]");
}

void  Server::saveUploadedFile_(const int fd)
{
  logger(LOG_DEBUG, "In function saveUploadedFile_");
  /*
  HTTP/1.1 201 Created
  Content-Type: text/plain
  Content-Length: 29

  File uploaded successfully.
  */
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
}

void  Server::respondMissingUploadDir(const int fd)
{
  logger(LOG_DEBUG, "In function respondMissingUploadDir");
  /*
   HTTP/1.1 500 Internal Server Error
  Content-Type: text/plain
  Content-Length: 49

  Server misconfiguration: upload_dir not specified.
  */
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
}


void  Server::DELETEMethod_(std::string& uri, const int fd)
{
  std::cout << "On delete method" << std::endl;
  LocationConfig  location = this->getCurrentLocation();
  std::string     localPath;
  std::string     extractUri;
  localPath = location.root + '/' + uri.substr(location.path.size());
  this->_localPath = localPath;
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
      this->cannotDeleteFile_(fd, localPath);
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

void  Server::cannotDeleteFile_(const int fd, std::string& path)
{
  logger(LOG_DEBUG, "In function cannotDeleteFile_");
  /*
  HTTP/1.1 409 Conflict
  Content-Type: text/plain
  Content-Length: 65

  Conflict: Unable to delete '/path/to/file' due to current resource state
  */
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(409, fd);
  if (this->hasCustomErrorPage(409, fd))
    this->saveErrorBodyFilePath(409, fd, contentType, contentLength);
  else
  {
    body = "Conflict: Unable to delete '" + path + "' due to current resource state";
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
}

void  Server::respondNotFound_(const int fd)
{
  logger(LOG_DEBUG, "In function respondNotFound_");
  /*
  HTTP/1.1 404 Not Found
  Content-Type: text/plain
  Content-Length: 23

  404 Not Found: Invalid path.
  */
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
}


void  Server::respondDirectoryListingForbidden(const int fd)
{
  logger(LOG_DEBUG, "In function respondDirectoryListingForbidden");
  /*
  HTTP/1.1 403 Forbidden
  Content-Type: text/plain
  Content-Length: 35

  403 Forbidden: Directory listing denied.
  */
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
}

void  Server::respondFileNotReadable(const int fd)
{
  logger(LOG_DEBUG, "In function respondFileNotReadable");
  /*
  HTTP/1.1 403 Forbidden
  Content-Type: text/plain
  Content-Length: 48

  You do not have permission to read the requested file.
  */
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
}

void  Server::respondDeleteDirConflict_(const int fd)
{
  logger(LOG_DEBUG, "In function respondDeleteDirConflict_");
  /*
  HTTP/1.1 409 Conflict
  Content-Type: text/plain
  Content-Length: XX

  Cannot delete resource due to a conflict with the current state.
  */
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;


  this->setStatus(409, fd);
  if (this->hasCustomErrorPage(409, fd))
    this->saveErrorBodyFilePath(409, fd, contentType, contentLength);
  else
  {
    body = "Cannot delete resource due to a conflict with the current state.";
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
}

// TODO Zramahaz
void  Server::buildDirectoryListing_(const int fd)
{
  logger(LOG_DEBUG, "In function buildDirectoryListing_");
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

void  Server::processReadableFile_(const int fd, const std::string& path)
{
  logger(LOG_DEBUG, "In function processReadableFile_");
  std::string contentType = this->getContentTypeByFileExtension(path);
  std::cout << "value of Content-Type " << contentType << std::endl;

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
  this->openAndSaveBodyFileFd(path, fd);
}

void  Server::onDeleteSuccess_(const int fd)
{
  logger(LOG_DEBUG, "In function onDeleteSuccess_");
  /*
  HTTP/1.1 204 No Content
  Content-Length: 0
  */

  this->setStatus(204, fd);

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 204 No Content\r\n"
    << CL << " " << "0" << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody("");
}

void  Server::methodNotAllowed_(const int fd)
{
  logger(LOG_DEBUG, "In function methodNotAllowed_");
  /*
  HTTP/1.1 405 Method Not Allowed
  Allow: 
  Content-Type: text/plain
  Content-Length: 46
  Connection: 
  The method GET is not allowed on /restricted.
  */
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
  /*
  HTTP/1.1 400 Bad Request
  Content-Type: text/plain
  Content-Length: 42

  400 Bad Request: Invalid HTTP request syntax.
  */
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
}

void  Server::handleNoMatchingLocation_(const int fd)
{

  logger(LOG_DEBUG, "In function handleNoMatchingLocation_");
  /*
   HTTP/1.1 404 Not Found
  Content-Type: text/plain
  Content-Length: 48
  Connection: close

  404 Not Found: The requested resource does not exist.
  */
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
}

// TODO need more test
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
        this->openAndSaveBodyFileFd(path, fd);
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
        this->openAndSaveBodyFileFd(path, fd);
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
        //return "Connection: " + it->second + "\r\n\r\n";
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
}

void  Server::respondNotImplemented_(const int& fd)
{
  logger(LOG_DEBUG, "In function respondNotImplemented_");
  /*
  HTTP/1.1 501 Not Implemented
  Content-Type: text/plain
  Content-Length: 35
  Connection: close

  501 Not Implemented: Unsupported return code
  */
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(501, fd);
  if (this->hasCustomErrorPage(501, fd))
    this->saveErrorBodyFilePath(501, fd, contentType, contentLength);
  else
  {
    body = "501 Not Implemented: Unsupported return code";
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
}

bool  Server::isRedirectCode_(int statusCode)
{
    return (statusCode >= 300 && statusCode < 400);
}

void  Server::respondRedirect_(const int& fd,
    const std::map<int, std::string>::const_iterator it)
{
  logger(LOG_DEBUG, "In function respondRedirect_");
  /*
  HTTP/1.1 [Code] Moved Permanently
  Location: http://example.com/new-path
  Content-Length: 0
  Connection: close
  */
  std::ostringstream headers;

  headers << this->getVersion(fd) << " " << it->first << " Moved Permanently\r\n"
    << "Location: " << it->second << "\r\n"
    << CL << " " << "0" << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());

  this->setStatus(it->first, fd);
}

void  Server::handleReturnWithoutUrl_(const int& fd)
{
  logger(LOG_DEBUG, "In function handleReturnWithoutUrl");
  /*
  HTTP/1.1 403 Forbidden
  Content-Type: text/plain
  Content-Length: 24

  Access denied: Forbidden.
  */
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
}

bool  Server::isValidHttpStatusCode_(const int& code)
{
  return (code >= 100 && code <= 599);
}

void  Server::setBodyFilePath(const int& fd, const std::string& path)
{
  this->_clients[fd]->getResponse().setBodyFilePath(path);
}

void  Server::createAndSaveRootLocation_(ServerConfigConstIterator& cfg)
{
  LocationConfig  rootLocation;

  for (std::vector<std::string>::const_iterator it = cfg->indexs.begin();
      it != cfg->indexs.end(); it++)
    rootLocation.indexs.push_back((*it));

  for (std::vector<std::string>::const_iterator it = cfg->methods.begin();
      it != cfg->methods.end(); it++)
    rootLocation.methods.push_back((*it));

  for (std::map<int, std::string>::const_iterator it = cfg->error_pages.begin();
      it != cfg->error_pages.end(); it++)
    rootLocation.error_pages[it->first] = it->second;

  rootLocation.autoindex = cfg->autoindex;
  rootLocation.client_max_body_size = cfg->client_max_body_size;
  rootLocation.upload_dir = cfg->upload_dir;
  rootLocation.path = "root";
  rootLocation.root = cfg->root;
  this->setCurrentLocation(rootLocation);
}

void  Server::openAndSaveBodyFileFd(const std::string& path, const int& clientFd)
{
  logger(LOG_DEBUG, "Attempting to open the file and store its file descriptor...");
  int fd;

  fd = open(path.c_str(), O_RDONLY);
  if (fd == -1)
    logger(LOG_FATAL,
        "Warning: The file should be readable but an issue was detected.");
  this->_clients[clientFd]->getResponse().setBodyFileFd(fd);
}

void  Server::setBodySize(const int& fd, const ssize_t& bodySize)
{
  this->_clients[fd]->getResponse().setBodySize(bodySize);
}

void  Server::respondPayloadTooLarge(const int& fd)
{
  logger(LOG_DEBUG, "In function respondPayloadTooLarge");
  /*
  HTTP/1.1 413 Request Entity Too Large
  Content-Type: text/plain
  Content-Length: 42

  413 Request Entity Too Large
  */
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
}
