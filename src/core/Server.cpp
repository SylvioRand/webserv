/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 10:26:47 by srandria          #+#    #+#             */
/*   Updated: 2025/07/21 17:32:33 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/Server.hpp"

Server::Server(const Config& config) : _config(config)
{
  this->start_server_();
}

Server::~Server(void)
{

}

// TODO
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
        accept_new_client_(fd);
      }
      else if (_clients.find(fd) != _clients.end())
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
    logger(LOG_INFO, "Coucouuuu");
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
    this->registerListenerToPoll_(fd);
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

  oss << "🟢 listening on  " << cfg.host << ":" << cfg.port;
  logger(LOG_INFO, oss.str());

  _listener_fds.push_back(fd);
}

void  Server::registerListenerToPoll_(int fd)
{
  struct pollfd pfd;
  pfd.fd = fd;
  pfd.events = POLLIN;
  pfd.revents = 0;            // poll() will overwrite this value when an event is captured.

  _pool_fds.push_back(pfd);
}

// TODO
void  Server::accept_new_client_(int listener_fd)
{
  (void)listener_fd;
}

// TODO
void  Server::handle_pollin_(int fd)
{
  (void)fd;
}

// TODO
void  Server::handle_pollout_(int fd)
{
  (void)fd;
}

// TODO
void  Server::close_client_(int fd)
{
  close(fd);
}

// TODO
void  Server::check_timout_(void)
{

}
