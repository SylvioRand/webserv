/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:24:24 by srandria          #+#    #+#             */
/*   Updated: 2025/07/25 16:03:58 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/Client.hpp"
#include <cerrno>
#include <cstddef>
#include <sys/socket.h>
#include <sys/types.h>

Client::Client(int fd) : _fd(fd), _lastActivity(time(NULL))
{
}

Client::~Client(void)
{
  if (_fd != -1) {
    close(_fd);
  }
}

void  Client::readData(void)
{
  char buf[4096];
  ssize_t bytes;
  bytes = recv(_fd, buf, sizeof(buf), 0);

  std::cout << buf << std::endl;
  if (bytes == -1)
  {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      logger(LOG_DEBUG, "end of reading content on fd");
      return ;
    }
    logger(LOG_ERROR, "error while reading fd with recv [at readData(void) function]");
    return ;
  }
  if (bytes == 0)
  {
    logger(LOG_INFO, "the client has closed the connection");
    return ;
  }
  this->_buffer.append(_buffer);
}

// TODO
void  Client::sendData(void)
{

}

bool  Client::isRequestComplete(void) const
{
  return (_request.isComplete());
}
