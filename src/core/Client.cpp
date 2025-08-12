/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:24:24 by srandria          #+#    #+#             */
/*   Updated: 2025/08/12 15:35:36 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/Client.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


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

bool  Client::readData(void)
{
  char buf[8192];
  ssize_t bytes;
  bytes = recv(_fd, buf, sizeof(buf), 0);

  std::cout << buf << std::endl;
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
    this->_buffer.append(buf);
    this->_request.parse(_buffer);
  }
  else if (this->_request.getMethod() == "POST" && !this->_request.isComplete())
  {
    logger(LOG_INFO, "POST detected");
    if (this->getRequest().isBodySizeAllowed())
      this->_request.appendToBody(buf);
    else
      this->_request.isComplete();
  }
  return (true);
}

// TODO
void  Client::sendData(std::string &localPath)
{
  (void)localPath;
  logger(LOG_INFO, "Sending Data ...");
  logger(LOG_DEBUG, "Status value [" + toString(this->_response.getStatus()) + "]");
  if (!this->_response.areHeadersFullySent())
    this->_response.sendHeaders(this->_fd);
  else if (!this->_response.isBodyFullySent())
    this->_response.sendBody(this->_fd);

  // just to verify
  std::cout << "------------------------------------" << std::endl;
  std::cout << this->_response.build() << std::endl;
  std::cout << "------------------------------------" << std::endl;

  std::cout << "=======================" << std::endl;
  std::cout << "file body path [" << this->_response.getBodyFilePath()
    << "]"<< std::endl;
  std::cout << "file body fd [" << this->_response.getBodyFileFd()
    << "]"<< std::endl;
  std::cout << "=======================" << std::endl;
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
