/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:39:05 by srandria          #+#    #+#             */
/*   Updated: 2025/08/08 14:24:42 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/HttpResponse.hpp"
#include <string>
#include <sys/socket.h>

HttpResponse::HttpResponse(void) : _status_code(200), _bodyFileFd(-1), _headersSize(0), 
  _bodySize(0), _headersOffset(0), _bodyOffset(0)
{
}


HttpResponse::~HttpResponse(void)
{
}

void  HttpResponse::setStatus(int code)
{
  _status_code = code;
}

void  HttpResponse::setBody(const std::string &content)
{
  this->_body= content;
}

void  HttpResponse::setHeader(const std::string &content)
{
  this->_headers = content;
}


// TODO
void  HttpResponse::sendFile(const std::string &path)
{
  (void)path;
}

std::string HttpResponse::build(void) const
{
  return (this->_headers + "\r\n" + this->_body);
}

int   HttpResponse::getStatus(void) const
{
  return (_status_code);
}

void  HttpResponse::saveHeadersAndBodySize(void)
{
  if (!this->_bodySize)
    this->_bodySize = this->_body.size();
  this->_headersSize = this->_headers.size();
}

// TODO
void HttpResponse::sendHeaders(const int& fd)
{
  logger(LOG_DEBUG, " -> Sendind HEADERS <-");
  ssize_t bytesSent = send(fd, this->_headers.data() + this->_headersOffset,
      _headers.size() - this->_headersOffset, 0);
  if (bytesSent > 0)
    this->_headersOffset += bytesSent;
}

// TODO
void HttpResponse::sendBody(const int& fd)
{
  (void)fd;
  logger(LOG_DEBUG, " -> Sending BODY <-");
  if (this->_bodyFilePath.empty())
  {
    logger(LOG_DEBUG, "you can directly send() body");
  }
  else if (this->_bodyFileFd != -1)
  {
    // TODO we need to close the client here
    logger(LOG_WARNING, "Can't open the file to serve in HTTP response body.");
  }
  else
  {
    logger(LOG_DEBUG, "you can read() and send() body file content");
  }
}

bool  HttpResponse::areHeadersFullySent(void)
{
  if (this->_headersOffset == this->_headersSize)
    return (true);
  return (false);
}

bool  HttpResponse::isBodyFullySent(void)
{
  if (this->_bodyOffset == this->_bodySize)
    return (true);
  return (false);
}

void  HttpResponse::setBodyFilePath(const std::string path)
{
  this->_bodyFilePath = path;
}

void  HttpResponse::setBodyFileFd(const int& fd)
{
  this->_bodyFileFd = fd;
}

std::string& HttpResponse::getBodyFilePath(void)
{
  return (this->_bodyFilePath);
}

int& HttpResponse::getBodyFileFd(void)
{
  return (this->_bodyFileFd);
}
