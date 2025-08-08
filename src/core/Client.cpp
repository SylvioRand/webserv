/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:24:24 by srandria          #+#    #+#             */
/*   Updated: 2025/08/08 15:06:50 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/Client.hpp"
#include <cerrno>
#include <cstddef>
#include <sys/socket.h>
#include <sys/types.h>

Client::Client(int fd, ServerConfigConstIterator cfg) : _fd(fd),
  _lastActivity(time(NULL)), _cfg(cfg)
{
}

// TODO Make sure we handle everything to avoid memory leaks
Client::~Client(void)
{
  if (_fd != -1)
  {
    close(_fd);
  }
}

void  Client::readData(void)
{
  char buf[8192];
  ssize_t bytes;
  bytes = recv(_fd, buf, sizeof(buf), 0);

  std::cout << buf << std::endl;
  if (bytes == -1)
  {
    if (errno == EAGAIN || errno == EWOULDBLOCK)    // interdit selon le suket
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
}

// TODO by zramahaz
void  Client::sendData(std::string &localPath)
{
  (void)localPath;
  logger(LOG_INFO, "Sending Data ...");
    while (1)
      ;
  if (this->_response.getStatus() == 200)
  {
    std::cout << "Status value [" << this->_response.getStatus() << std::endl;
    while (1)
      ;
  }
}

bool  Client::isRequestComplete(void) const
{
  return (_request.isComplete());
}

const HttpRequest& Client::getRequest(void) const
{
  return (_request);
}


HttpResponse& Client::getResponse(void)
{
  return (_response);
}

Client::ServerConfigConstIterator Client::getServerConfig(void) const
{
  return (_cfg);
}

