/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 10:26:47 by srandria          #+#    #+#             */
/*   Updated: 2025/07/21 08:52:26 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../../include/core/Server.hpp"

Server::Server(const Config& config) : _config(config)
{

}

// TODO
void  Server::start_server(void)
{

}

// TODO
void  Server::stop_server(void)
{

}

// TODO
void  Server::create_all_listeners_(void)
{

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


/*
Server::Server(void) {}

Server::~Server(void)
{
  for (size_t i = 0; i < _listen_fds.size(); ++i)
  {
    if (_listen_fds[i].socketFd != -1)
      close(_listen_fds[i].socketFd);
  }
}

void Server::addListen(const std::string &host, int port)
{
  ListenInfo info;
  info.host = host;
  info.port = port;
  info.socketFd = -1;
  info.family = AF_UNSPEC;
  std::memset(&info.address, 0, sizeof(info.address));
  info.addrLen = 0;
  _listen_fds.push_back(info);
}

void Server::initSockets()
{
  for (size_t i = 0; i < _listen_fds.size(); ++i)
  {
    ListenInfo &info = _listen_fds[i];

    // Résolution d'adresse générique (IPv4 ou IPv6)
    struct addrinfo hints;
    struct addrinfo *res;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // Supporte IPv4 et IPv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    char portStr[6];
    snprintf(portStr, sizeof(portStr), "%d", info.port);

    int status = getaddrinfo(info.host.c_str(), portStr, &hints, &res);
    if (status != 0)
    {
      std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
      throw std::runtime_error("Invalid host: " + info.host);
    }

    info.family = res->ai_family;
    info.addrLen = res->ai_addrlen;
    std::memcpy(&info.address, res->ai_addr, res->ai_addrlen);

    info.socketFd = socket(info.family, SOCK_STREAM, 0);
    if (info.socketFd < 0)
    {
      std::cerr << "socket: " << strerror(errno) << std::endl;
      freeaddrinfo(res);
      throw std::runtime_error("Failed to create socket");
    }

    // Non bloquant
    if (fcntl(info.socketFd, F_SETFL, O_NONBLOCK) < 0)
    {
      std::cerr << "fcntl: " << strerror(errno) << std::endl;
      freeaddrinfo(res);
      throw std::runtime_error("Failed to set socket to non-blocking");
    }

    int opt = 1;
    if (setsockopt(info.socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
      std::cerr << "setsockopt: " << strerror(errno) << std::endl;
      freeaddrinfo(res);
      throw std::runtime_error("Failed to set socket options");
    }

    // Bind socket
    if (bind(info.socketFd, (struct sockaddr *)&info.address, info.addrLen) < 0)
    {
      std::cerr << "bind: " << strerror(errno) << std::endl;
      freeaddrinfo(res);
      throw std::runtime_error("Bind failed");
    }

    freeaddrinfo(res);
  }
}

void Server::startListening()
{
  for (size_t i = 0; i < _listen_fds.size(); ++i)
  {
    if (listen(_listen_fds[i].socketFd, SOMAXCONN) < 0)
    {
      std::cerr << "listen: " << strerror(errno) << std::endl;
      throw std::runtime_error("Listen failed");
    }

    std::cout << "🟢 Listening on "
              << _listen_fds[i].host << ":" << _listen_fds[i].port
              << ((_listen_fds[i].family == AF_INET6) ? " (IPv6)" : " (IPv4)")
              << std::endl;
  }
}

const std::vector<int>& Server::getListenFds() const
{
  static std::vector<int> fds;
  fds.clear();
  for (size_t i = 0; i < _listen_fds.size(); ++i)
    fds.push_back(_listen_fds[i].socketFd);
  return fds;
}
*/
