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

HttpResponse::HttpResponse(void)
    : _status_code(200), _bodyFileFd(-1), _headersSize(0), _headersOffset(0),
      _bodySize(0), _bufferSize(0), _bufferOffset(0), _bodyBytesSent(0),
      _keepAlive(true) {}

void HttpResponse::initializeState(void)
{
  static int value = 0;
  value++;
  logger(LOG_DEBUG, "***********initializeState in HttpResponse*************");
  if (this->_bodyFileFd != -1)
  {
    logger(LOG_DEBUG, "the body fd file will be close()");
    close(this->_bodyFileFd);
  }
  this->_bodyFileFd = -1;
  this->_bodyFilePath.clear();
  this->_headersSize = 0;
  this->_headersOffset = 0;
  this->_bodySize = 0;
  this->_bufferSize = 0;
  this->_bufferOffset = 0;
  this->_bodyBytesSent = 0;
  this->_keepAlive = true;
}

HttpResponse::~HttpResponse(void) {}

void HttpResponse::setStatus(int code) { _status_code = code; }

void HttpResponse::setBody(const std::string &content) {
  this->_body = content;
}

void HttpResponse::setHeader(const std::string &content) {
  this->_headers = content;
}

std::string HttpResponse::build(void) const {
  return (this->_headers + "\r\n" + this->_body);
}

int HttpResponse::getStatus(void) const { return (_status_code); }

void HttpResponse::saveHeadersAndBodySize(void) {
  if (!this->_bodySize)
    this->_bodySize = this->_body.size();
  this->_headersSize = this->_headers.size();
}

void HttpResponse::sendHeaders(const int &fd) {
  logger(LOG_DEBUG, " -> Sendind HEADERS <-");
  ssize_t bytesSent = send(fd, this->_headers.data() + this->_headersOffset,
                           _headers.size() - this->_headersOffset, 0);
  if (bytesSent > 0)
    this->_headersOffset += bytesSent;
}

// TODO
void HttpResponse::sendBody(const int &fd)
{
  logger(LOG_DEBUG, " -> Sending BODY <-");
  if (this->_bodyFilePath.empty()) {
    ssize_t bytesSent = send(fd, this->_body.data() + this->_bodyBytesSent,
                             this->_body.size() - this->_bodyBytesSent, 0);
    logger(LOG_DEBUG, "you can directly send() body");
    if (bytesSent > 0)
      this->_bodyBytesSent += bytesSent;
  } else if (this->_bodyFileFd == -1) {
    // TODO we need to close the client here
    logger(LOG_WARNING, "Can't open the file to serve in HTTP response body.");
  } else {
    logger(LOG_DEBUG, "you can read() and send() body file content");
    if (this->_bufferOffset == this->_bufferSize) {
      this->_bufferOffset = 0;
      logger(LOG_DEBUG, "reading file ...");
      ssize_t bytesRead;
      bytesRead = read(this->_bodyFileFd, this->_bodyBuffer, READ_CHUNK_SIZE);
      if (bytesRead > 0) {
        logger(LOG_DEBUG, "Sending after reading file ...");
        this->_bufferSize = bytesRead;
        ssize_t bytesSent = send(fd, this->_bodyBuffer + this->_bufferOffset,
                                 this->_bufferSize - this->_bufferOffset, 0);
        if (bytesSent > 0) {
          logger(LOG_DEBUG, "bytesSent > 0");
          this->_bodyBytesSent += bytesSent;
          this->_bufferOffset += bytesSent;
        } else if (bytesSent == 0)
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
      if (bytesSent > 0) {
        logger(LOG_DEBUG, "bytesSent > 0");
        this->_bodyBytesSent += bytesSent;
        this->_bufferOffset += bytesSent;
      }
      else if (bytesSent == 0)
        logger(LOG_DEBUG, "bytesSent == 0");
      else if (bytesSent == -1)
        logger(LOG_DEBUG, "bytesSent == -1");
    }
  }
  std::cout << "on ressort **************" << std::endl;
}

bool HttpResponse::areHeadersFullySent(void) {
  logger(LOG_DEBUG, "verifying areHeadersFullySent");
  std::cout << "---------------" << std::endl;
  std::cout << "headers size = " << this->_headersSize << std::endl;
  std::cout << "headers offser = " << this->_headersOffset << std::endl;
  std::cout << "body size = " << this->_bodySize << std::endl;
  std::cout << "body offset = " << this->_bodyBytesSent << std::endl;
  std::cout << "---------------" << std::endl;
  if (this->_headersOffset == this->_headersSize) {
    logger(LOG_DEBUG, "Headers are fully sent");
    return (true);
  }
  logger(LOG_DEBUG, "Headers are not fully sent");
  return (false);
}

bool HttpResponse::isBodyFullySent(void) {
  logger(LOG_DEBUG, "verifying isBodyFullySent");

  if (this->_bodySize == 0)
    return (true);
  if (this->_bodyBytesSent == this->_bodySize && this->_bodySize != 0) {
    logger(LOG_DEBUG, "Body is fully sent");
    return (true);
  }
  logger(LOG_DEBUG, "Body is not fully sent");
  return (false);
}

void HttpResponse::setBodyFilePath(const std::string path) {
  logger(LOG_DEBUG, "verifying areHeadersFullySent");
  this->_bodyFilePath = path;
}

void HttpResponse::setBodyFileFd(const int &fd) { this->_bodyFileFd = fd; }

std::string &HttpResponse::getBodyFilePath(void) {
  return (this->_bodyFilePath);
}

int &HttpResponse::getBodyFileFd(void) { return (this->_bodyFileFd); }

void HttpResponse::setBodySize(const ssize_t &bodySize) {
  this->_bodySize = bodySize;
}

void HttpResponse::setKeepAliveStatus(bool value) { this->_keepAlive = value; }

bool HttpResponse::isKeepAlive(void) { return (this->_keepAlive); }
