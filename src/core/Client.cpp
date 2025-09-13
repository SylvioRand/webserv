/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:24:24 by srandria          #+#    #+#             */
/*   Updated: 2025/09/13 16:54:33 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/Client.hpp"
#include <cstddef>
#include <cstdio>
#include <ctime>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


Client::Client(int fd, ServerConfigConstIterator cfg) : _isReadingCgiResponse(false),
  _fd(fd), _lastActivity(time(NULL)), _cfg(cfg), _path(""), _childPid(0)
{
  this->_buffer.clear();
}


Client::~Client(void)
{
  if (_fd != -1)
  {
    close(_fd);
  }
}

pid_t&  Client::getChildPid(void)
{
  return (this->_childPid);
}

void  Client::setLastActivity(void)
{
  this->_lastActivity = time(NULL);
}

void  Client::setChildPid(const pid_t& childPid)
{
  this->_childPid = childPid;
}

time_t&  Client::getLastActivity(void)
{
  return (this->_lastActivity);
}

bool  Client::readData(void)
{
  char buf[READ_CHUNK_SIZE];
  ssize_t bytes;
  bytes = recv(_fd, buf, sizeof(buf), 0);
  if (bytes == -1)
  {
    logger(LOG_INFO, "[RECV] Failed to receive data from client");
    return (false);
  }
  else if (bytes == 0)
    return (false);

  this->setLastActivity();
  if (this->_request.getMethod() == "POST" && !this->_request.isComplete())
  {
    this->_request.extractRequestBody(buf, bytes);
  }
  else if (this->_request.getMethod().empty())
  {
    if (!this->_request._isReadingRequest)
    {
      logger(LOG_INFO,
          "[HTTP] Reading request data from socket (recv) from client fd=" + toString(this->_fd) + " ...");
      this->_request._isReadingRequest = true;
      this->_request.setServerConf(this->_cfg);
    }
    this->_buffer.append(buf, bytes);
    this->_request.parse(_buffer);
  }
  return (true);
}

void  Client::sendData(void)
{
  if (!this->_response._isSending)
  {
    logger(LOG_INFO, "📤 Sending HTTP response to client fd=" + toString(this->_fd) + " ...");
    this->_response._isSending = true;
  }
  if (!this->_response.areHeadersFullySent())
    this->_response.sendHeaders(this->_fd);
  else if (!this->_response.isBodyFullySent())
    this->_response.sendBody(this->_fd);
}

void  Client::sendCgiData(void)
{
  if (!this->_response._isSending)
  {
    logger(LOG_INFO, "📤 Sending HTTP cgi response to client fd=" + toString(this->_fd) + " ...");
    this->_response._isSending = true;
  }
  this->_response.sendCgiResponse(this->_fd);
}

void  HttpResponse::sendCgiResponse(const int&fd)
{
  ssize_t bytesSent = send(fd, this->_cgiResponse.data() + this->_cgiBytesSent,
                           this->_cgiResponse.size() - this->_cgiBytesSent, 0);
  if (bytesSent > 0)
    this->_cgiBytesSent += bytesSent;
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

void  Client::setPath(const std::string path)
{
  this->_path = path;
}

const std::string&  Client::getPath()
{
  return (this->_path);
}

void  Client::setCurrentLocation(LocationConfig& location)
{
  this->_currentLocation = location;
}

