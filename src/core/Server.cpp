#include "../../include/core/Server.hpp"
#include <stdexcept>
#include <sys/socket.h>

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
  _address.sin_addr.s_addr = INADDR_ANY

  if (bind(_socketFd, (struct sockaddr *)&_address, sizeof(_address)) < 0) {
        perror("bind");
        throw std::runtime_error("Bind failed");
    }
}
