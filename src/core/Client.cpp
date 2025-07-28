/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:24:24 by srandria          #+#    #+#             */
/*   Updated: 2025/07/28 16:51:21 by srandria         ###   ########.fr       */
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
  if (_fd != -1)
  {
    close(_fd);
  }
}

void  Client::readData(void)
{
  char buf[8];
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
  if (!this->_request.getMethod().size())
  {
    this->_buffer.append(buf);
    this->_request.parse(_buffer);
  }
  else if (this->_request.getMethod() == "POST" && !this->_request.isComplete())
  {
    logger(LOG_INFO, "POST detected");
    this->_request.appendToBody(buf);
  }

  // TODO remove this on production
  if (this->_request.isComplete())
  {
    logger(LOG_INFO, "request is completed");
    logger(LOG_INFO, "print body \n" + this->_request.getBody());
  }
}

// TODO
void  Client::sendData(void)
{

}

bool  Client::isRequestComplete(void) const
{
  return (_request.isComplete());
}
