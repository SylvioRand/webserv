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
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <sys/socket.h>

Server::Server(int port, const std::string &host) : _port(port), _host(host),
  _socketFd(-1)
{
  std::memset(&_address, 0, sizeof(_address));
}

Server::~Server(void)
{
  if (_socketFd != -1)
    close(_socketFd);
}

void  Server::initSocket(void)
{
  _socketFd = socket(AF_INET, SOCK_STREAM, 0);
  if (_socketFd < 0) {
    std::cerr << "socket: " << strerror(errno) << std::endl;
    throw std::runtime_error("Failed to create socket");
  }

  // Configurer le socket en mode non bloquant
  if (fcntl(_socketFd, F_SETFL, O_NONBLOCK) < 0) {
    std::cerr << "fcntl: " << strerror(errno) << std::endl;
    throw std::runtime_error("Failed to set socket to non-blocking");
  }

  int opt = 1;
  if (setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    std::cerr << "setsockopt: " << strerror(errno) << std::endl;
    throw std::runtime_error("Failed to set socket options");
  }

  _address.sin_family = AF_INET;
  _address.sin_port = htons(_port);
  if (_host == "0.0.0.0") {
    _address.sin_addr.s_addr = INADDR_ANY;
  }
  else {
    struct addrinfo hints;
    struct addrinfo *res;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;


    int status = getaddrinfo(_host.c_str(), NULL, &hints, &res);
    if (status != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
        throw std::runtime_error("Invalid host: " + _host);
    }

    struct sockaddr_in *addr_in = (struct sockaddr_in *)res->ai_addr;
    _address.sin_addr = addr_in->sin_addr;

    freeaddrinfo(res);

  }
  if (bind(_socketFd, (struct sockaddr *)&_address, sizeof(_address)) < 0){
    std::cerr << "bind: " << strerror(errno) << std::endl;
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


