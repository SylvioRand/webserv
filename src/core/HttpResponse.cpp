/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:39:05 by srandria          #+#    #+#             */
/*   Updated: 2025/08/13 11:51:53 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/HttpResponse.hpp"
#include <cmath>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

HttpResponse::HttpResponse(void)
    : _status_code(200), _bodyFileFd(-1), _headersSize(0), _headersOffset(0),
      _bodySize(0), _bufferSize(0), _bufferOffset(0), _bodyBytesSent(0),
      _keepAlive(true), _cgiBytesSent(0), _isFullySent(false) {}

void HttpResponse::initializeState(void)
{
  this->_bodyFileFd = -1;
  this->_bodyFilePath.clear();
  this->_headersSize = 0;
  this->_headersOffset = 0;
  this->_bodySize = 0;
  this->_bufferSize = 0;
  this->_bufferOffset = 0;
  this->_bodyBytesSent = 0;
  this->_keepAlive = true;
  this->_isFullySent = false;
  this->_cgiBytesSent = 0;
}

void  HttpResponse::closeBodyFileFd(const int&fd, const std::string path)
{
  if (this->_bodyFileFd != -1)
  {
    logger(LOG_INFO,
      "Closed file descriptor for response body: (previously associated with '"
      + path  + "') fd=" + toString(fd));
    close(this->_bodyFileFd);
  }
}

HttpResponse::~HttpResponse(void) {}

void HttpResponse::setStatus(int code)
{
  _status_code = code;
}

void HttpResponse::setBody(const std::string &content)
{
  this->_body = content;
}

void HttpResponse::setHeader(const std::string &content)
{
  this->_headers = content;
}

std::string HttpResponse::build(void) const
{
  return (this->_headers + "\r\n" + this->_body);
}

int HttpResponse::getStatus(void) const
{
  return (_status_code);
}

void HttpResponse::saveHeadersAndBodySize(void)
{
  if (!this->_bodySize)
    this->_bodySize = this->_body.size();
  this->_headersSize = this->_headers.size();
}

void HttpResponse::sendHeaders(const int &fd)
{
  ssize_t bytesSent = send(fd, this->_headers.data() + this->_headersOffset,
                           _headers.size() - this->_headersOffset, 0);
  if (bytesSent > 0)
    this->_headersOffset += bytesSent;
}

// TODO
void HttpResponse::sendBody(const int &fd)
{
  if (this->_bodyFilePath.empty()) {
    ssize_t bytesSent = send(fd, this->_body.data() + this->_bodyBytesSent,
                             this->_body.size() - this->_bodyBytesSent, 0);
    if (bytesSent > 0)
      this->_bodyBytesSent += bytesSent;
  }
  else if (this->_bodyFileFd == -1) {
    // TODO we need to close the client here
  }
  else
  {
    if (this->_bufferOffset == this->_bufferSize)
    {
      this->_bufferOffset = 0;
      ssize_t bytesRead;
      bytesRead = read(this->_bodyFileFd, this->_bodyBuffer, READ_CHUNK_SIZE);
      if (bytesRead > 0)
      {
        this->_bufferSize = bytesRead;
        ssize_t bytesSent = send(fd, this->_bodyBuffer + this->_bufferOffset,
                                 this->_bufferSize - this->_bufferOffset, 0);
        if (bytesSent > 0)
        {
          this->_bodyBytesSent += bytesSent;
          this->_bufferOffset += bytesSent;
        }
        else if (bytesSent == 0)
          logger(LOG_DEBUG, "bytesSent == 0");
        else if (bytesSent == -1)
          logger(LOG_DEBUG, "bytesSent == -1");
      }
      else 
      {
        // TODO we need potentially do something in this case
        logger(LOG_ERROR, "Warning, nothing has been read");
      }
    }
    else
    {
      logger(LOG_DEBUG, "reading file ...");
      ssize_t bytesSent = send(fd, this->_bodyBuffer + this->_bufferOffset,
                               this->_bufferSize - this->_bufferOffset, 0);
      if (bytesSent > 0)
      {
        this->_bodyBytesSent += bytesSent;
        this->_bufferOffset += bytesSent;
      }
      else if (bytesSent == 0)
        logger(LOG_DEBUG, "bytesSent == 0");
      else if (bytesSent == -1)
        logger(LOG_ERROR, "bytesSent == -1");
    }
  }
}

bool HttpResponse::areHeadersFullySent(void)
{
  if (this->_headersOffset == this->_headersSize)
  {
    return (true);
  }
  return (false);
}

bool HttpResponse::isBodyFullySent(void)
{
  if (this->_bodySize == 0)
    return (true);
  if (this->_bodyBytesSent == this->_bodySize && this->_bodySize != 0)
    return (true);
  return (false);
}

bool  HttpResponse::isCgiResponseFullySent(void)
{
  if (this->_cgiBytesSent == this->_cgiResponseSize)
    return (true);
  return (false);
}

void HttpResponse::setBodyFilePath(const std::string path)
{
  this->_bodyFilePath = path;
}

void HttpResponse::setBodyFileFd(const int &fd)
{
  this->_bodyFileFd = fd;
}

std::string &HttpResponse::getBodyFilePath(void)
{
  return (this->_bodyFilePath);
}

int &HttpResponse::getBodyFileFd(void)
{
  return (this->_bodyFileFd);
}

void HttpResponse::setBodySize(const ssize_t &bodySize)
{
  this->_bodySize = bodySize;
}

void HttpResponse::setKeepAliveStatus(bool value)
{
  this->_keepAlive = value;
}

bool HttpResponse::isKeepAlive(void)
{
  return (this->_keepAlive);
}

ssize_t HttpResponse::getCgiBytesSent(void)
{
  return (this->_cgiBytesSent);
}

ssize_t HttpResponse::getCgiRespondSize(void)
{
  return (this->_cgiResponseSize);
}

void HttpResponse::saveCgiRespondSize(const int& clientFd)
{
  this->_cgiResponseSize = this->_cgiResponse.size();
  std::string size = toString(this->_cgiResponseSize);
  logger(LOG_INFO,
    size + " bytes ready to be sent to client fd=" + toString(clientFd));
}

void  HttpResponse::appendCgiResponse(const std::string& buff, const ssize_t& size)
{
  this->_cgiResponse.append(buff.c_str(), size);
}

void  HttpResponse::sendCgiResponse(const int&fd)
{
  ssize_t bytesSent = send(fd, this->_cgiResponse.data() + this->_cgiBytesSent,
                           this->_cgiResponse.size() - this->_cgiBytesSent, 0);
  if (bytesSent > 0)
    this->_cgiBytesSent += bytesSent;
}

