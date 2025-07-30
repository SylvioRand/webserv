/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 10:26:47 by srandria          #+#    #+#             */
/*   Updated: 2025/07/29 16:54:10 by srandria         ###   ########.fr       */
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
  const std::vector<ServerConfig>& configs = _config.getServers();

  for (std::vector<ServerConfig>::const_iterator it = configs.begin(); it != configs.end(); ++it)
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
  int client_fd;
  struct sockaddr_in client_address;
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
    // TODO here we need to add something to prepare the _response (fill private variable)

    std::string uri = this->_clients[fd]->getRequest().getPath();
    logger(LOG_INFO, "the path " +  uri);
    this->buildLocalPath(uri, fd);
    this->setPollOut_(fd);
  }
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

  const std::map<std::string, std::string>& headers = _clients[fd]->getRequest().getHeaders();
  std::map<std::string, std::string>::const_iterator hostHeader = headers.find("Host");

  if (hostHeader == headers.end())
    throwWithLog(LOG_ERROR, "The Client Request doesn't have Host in _headers");

  std::string hostFromRequest = hostHeader->second;

  const std::vector<ServerConfig>&  servers = this->getConfig().getServers();
  for (std::vector<ServerConfig>::const_iterator cfg = servers.begin(); cfg != servers.end(); ++cfg)
  {
    std::string hostPort = (*cfg).host + ":" + intToString((*cfg).port);
    if (hostPort == hostFromRequest)
    {
      logger(LOG_INFO, "Config found for corresponding host -> " + hostFromRequest);
      path = (*cfg).root + uri;
      if (!path.empty() && path[path.length() - 1] == '/') {
        path += (*cfg).index;
      }
      break;
    }
  }
  if (path.empty())
      throwWithLog(LOG_ERROR, "No matching server configuration found for host: " + hostFromRequest);
  logger(LOG_DEBUG, "Local path [" + path + "]");
  return path;
}

// TODO
void  Server::check_timout_(void)
{

}

const Config& Server::getConfig(void)
{
  return (_config);
}

