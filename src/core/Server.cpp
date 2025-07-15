/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 10:26:47 by srandria          #+#    #+#             */
/*   Updated: 2025/07/15 10:42:09 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/Server.hpp"
#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>
#include <iostream>
#include <stdexcept>
#include <netdb.h>
#include <unistd.h>

Server::Server(void) {}

Server::~Server(void)
{
    for (size_t i = 0; i < _listens.size(); ++i)
    {
        if (_listens[i].socketFd != -1)
            close(_listens[i].socketFd);
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
    _listens.push_back(info);
}

void Server::initSockets()
{
    for (size_t i = 0; i < _listens.size(); ++i)
    {
        ListenInfo &info = _listens[i];

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
    for (size_t i = 0; i < _listens.size(); ++i)
    {
        if (listen(_listens[i].socketFd, SOMAXCONN) < 0)
        {
            std::cerr << "listen: " << strerror(errno) << std::endl;
            throw std::runtime_error("Listen failed");
        }

        std::cout << "🟢 Listening on "
                  << _listens[i].host << ":" << _listens[i].port
                  << ((_listens[i].family == AF_INET6) ? " (IPv6)" : " (IPv4)")
                  << std::endl;
    }
}

const std::vector<int>& Server::getListenFds() const
{
    static std::vector<int> fds;
    fds.clear();
    for (size_t i = 0; i < _listens.size(); ++i)
        fds.push_back(_listens[i].socketFd);
    return fds;
}
