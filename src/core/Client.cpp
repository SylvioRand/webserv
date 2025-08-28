/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:24:24 by srandria          #+#    #+#             */
/*   Updated: 2025/08/28 18:29:55 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/Client.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


Client::Client(int fd, ServerConfigConstIterator cfg) : _isReadingCgiResponse(false),
  _fd(fd), _lastActivity(time(NULL)), _cfg(cfg)
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

bool  Client::readData(void)
{
  logger(LOG_DEBUG, "[HTTP] Reading request data from socket (recv).");
  //char buf[8192];
  char buf[8192];
  ssize_t bytes;
  bytes = recv(_fd, buf, sizeof(buf), 0);
  if (bytes == -1)
  {
    logger(LOG_ERROR, "[RECV] Failed to receive data from client");
    return (false);
  }
  else if (bytes == 0)
  {
    logger(LOG_INFO, "Client closed the connection");
    return (false);
  }
  if (this->_request.getMethod().empty())
  {
    this->_buffer.append(buf, bytes);
    this->_request.parse(_buffer);
  }
  else if (this->_request.getMethod() == "POST" && !this->_request.isComplete())
  {
    std::string bodyPart;
    bodyPart.append(buf, bytes);
    this->_request.extractRequestBody(bodyPart);
  }
  return (true);
}

void  Client::sendData(void)
{
  logger(LOG_INFO, "📤 Sending HTTP response to client fd=" + toString(this->_fd) + " ...");
  if (!this->_response.areHeadersFullySent())
    this->_response.sendHeaders(this->_fd);
  else if (!this->_response.isBodyFullySent())
    this->_response.sendBody(this->_fd);
}

void  Client::sendCgiData(void)
{
  logger(LOG_INFO, "📤 Sending HTTP cgi response to client fd=" + toString(this->_fd) + " ...");
  this->_response.sendCgiResponse(this->_fd);
}

bool  Client::isRequestComplete(void) const
{
  return (_request.isComplete());
}

HttpRequest& Client::getRequest(void)
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

void  Client::clearBuffer(void)
{
  this->_buffer.clear();
}
