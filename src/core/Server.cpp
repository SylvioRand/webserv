/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 10:26:47 by srandria          #+#    #+#             */
/*   Updated: 2025/07/07 13:31:19 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/Server.hpp"
#include <stdexcept>
#include <sys/socket.h>

Server::Server(int port, const std::string &host) : _port(port), _host(host),
  _socketFd(-1)
{
  std::memset(&_address, 0, sizeof(_address));
}

Server::~Server(void)
{
  close(_socketFd);
}

void  Server::initSocket(void)
{
  _socketFd = socket(AF_INET, SOCK_STREAM, 0);
  if (_socketFd < 0)
  {
    std::cerr << "socket: " << strerror(errno) << std::endl;
    throw std::runtime_error("Failed to create socket");
  }

  int opt = 1;
  if (setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
  {
    std::cerr << "setsockopt: " << strerror(errno) << std::endl;
    throw std::runtime_error("Failed to set socket options");
  }

  _address.sin_family = AF_INET;
  _address.sin_port = htons(_port);
  _address.sin_addr.s_addr = INADDR_ANY;

  if (bind(_socketFd, (struct sockaddr *)&_address, sizeof(_address)) < 0)
  {
    std::cerr << "bing: " << strerror(errno) << std::endl;
    throw std::runtime_error("Bind failed");
  }
}

void  Server::startListening(void)
{
  if (listen(_socketFd, SOMAXCONN) < 0)
  {
    std::cerr << "listen: " << strerror(errno) << std::endl;
    throw std::runtime_error("Listen failed");
  }

  std::cout << "🟢 Listening on port " << _port << std::endl;
}

int  Server::getSocketFd(void)
{
  return (_socketFd);
}


