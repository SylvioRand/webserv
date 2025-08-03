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
#include <csignal>
#include <sstream>
#include <string>
#include <sys/socket.h>

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
  while (true)
  {
    int ready = poll(&_pool_fds[0], _pool_fds.size(), -1);
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
    if (!this->isMethodValid_(method))
      this->setStatus(405, fd);
    std::string uri = this->getUri_(fd);
    logger(LOG_INFO, "the path(uri) [" +  uri + "]");
    ServerConfigConstIterator serverConf = this->findMatchingServer(fd);
    logger(LOG_INFO, "the path(uri) [" +  uri + "]");
    saveMatchingLocation_(uri, serverConf);
///// here
    /*
    std::string localPath = this->_currentLocation.root;
    logger(LOG_DEBUG, "Local path [" + localPath + "]");
    */
    if (method == "GET")
    {    
      this->GETMethod(uri);
      /*
      else
      {
        if (this->getCurrentLocation().autoindex)
          this->setStatus(202, fd);
        else if (!this->getCurrentLocation().index.empty())
          this->setStatus(404, fd);
      }
      logger(LOG_DEBUG, "statuscode [" + intToString(this->_clients[fd]->getResponse().getStatus()) + "]");
      */
    }
    else if (method == "POST")
    {
      // if (getCurrentLocation().upload_dir.empty() )

    }
    else if (method == "DELETE")
    {

    }
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

void  Server::error405(int fd)
{
  (void)fd;
}

bool  Server::isMethodValid_(std::string method)
{
  return (method == "GET" || method == "POST" || method == "DELETE");
}


/*
// Dans la classe Server
void Server::resolveTarget(Client& client,
                           const std::string& localPath,
                           const Location& loc)
{
    struct stat st;

    if (stat(localPath.c_str(), &st) != 0) {
        client.getResponse().setStatus(404);
        return;
    }

    if (S_ISDIR(st.st_mode)) {
        if (!handleDirectory(localPath, loc, &client))
            return;            // 403 ou 404 déjà placé
    } else if (S_ISREG(st.st_mode)) {
        if (!fileOk(localPath))
            client.getResponse().setStatus(403);
        else
            client.getResponse().setFileToServe(localPath); // 200 OK
    } else {
        client.getResponse().setStatus(404); // ni fichier ni dossier
    }
}
*/

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
    } } logger(LOG_ERROR, "fd not found"); }
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

std::string Server::buildLocalPath(const std::string& uri, int fd)
{
  std::string                       path;

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
      saveMatchingLocation_(uri, cfg);
      if (this->getCurrentLocation().root.empty())
      {
        this->setStatus(404, fd);
        return ("");
      }
      path += this->getCurrentLocation().root + uri;
      if (!path.empty() && path[path.length() - 1] == '/') {
        path += cfg->index;
      }
      std::vector<ServerConfig>::const_iterator newcfg = cfg;
      (void)newcfg;
      break;
    }
  }
  if (path.empty())
      throwWithLog(LOG_ERROR, "No matching server configuration found for host: " + hostFromRequest);
  return (path);
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

bool  Server::fileOk(const std::string localPath) const
{
  struct stat st;
  return (stat(localPath.c_str(), &st) == 0 &&
          S_ISREG(st.st_mode) &&
          access(localPath.c_str(), R_OK) == 0);
}


void  Server::setCurrentLocation(LocationConfig& location)
{
  this->_currentLocation = location;
}
/*
bool  Server::filOk(const std::string localPath) const
{
  struct stat st;

  if (stat(localPath.c_str(), &st) != 0)
  {
    std::map<int, Client*>::const_iterator it = _clients.find(fd);
    if (it == _clients.end())
      throwWithLog(LOG_FATAL, "client not found for setting up status code");
    Client* client = it->second;
    client->getResponse().setStatus(404);
    logger(LOG_DEBUG, "stat failed");
    return (false);
  }
  // Verify that it’s a file and not a directory.
  if (!S_ISREG(st.st_mode))
  {
    std::map<int, Client*>::const_iterator it = _clients.find(fd);
    if (it == _clients.end())
      throwWithLog(LOG_FATAL, "client not found for setting up status code");
    Client* client = it->second;
    client->getResponse().setStatus(403);
    logger(LOG_DEBUG, "not a regular file");
    return (false);
  }
  // Check read permission
  if (access(localPath.c_str(), R_OK) != 0)
    return (false);
  return (true);
}
*/

// Dans la classe Server
/*
void Server::resolveTarget(Client& client, const std::string& localPath, const Location& loc)
{
    struct stat st;

    if (stat(localPath.c_str(), &st) != 0) {
        client.getResponse().setStatus(404);
        return;
    }

    if (S_ISDIR(st.st_mode)) {
        if (!handleDirectory(localPath, loc, &client))
            return;            // 403 ou 404 déjà placé
    } else if (S_ISREG(st.st_mode)) {
        if (!fileOk(localPath))
            client.getResponse().setStatus(403);
        else
            client.getResponse().setFileToServe(localPath); // 200 OK
    } else {
        client.getResponse().setStatus(404); // ni fichier ni dossier
    }
}
*/

void  Server::GETMethod(std::string& uri)
{
  LocationConfig  location = this->getCurrentLocation();
  std::string     path;
  std::string     extractUri;
  path = location.root + '/' + uri.substr(location.path.size());
  logger(LOG_DEBUG, "value of path [" + path + "]");
}
