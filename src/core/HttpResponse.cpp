/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:39:05 by srandria          #+#    #+#             */
/*   Updated: 2025/09/13 16:19:33 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/HttpResponse.hpp"
#include <cmath>
#include <ctime>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>

HttpResponse::HttpResponse(void)
    : _status_code(200), _headersSize(0), _headersOffset(0),
      _bodySize(0), _bufferSize(0), _bufferOffset(0), _bodyBytesSent(0),
      _keepAlive(true), _cgiBytesSent(0), _streamOffset(0), _isSending(false),
      _isFullySent(false)
{}

void HttpResponse::initializeState(void)
{
  this->_bodyFilePath.clear();
  this->_bodyFileStream.close();
  this->_body.clear();
  this->_headers.clear();
  this->_headersSize = 0;
  this->_headersOffset = 0;
  this->_bodySize = 0;
  this->_bufferSize = 0;
  this->_bufferOffset = 0;
  this->_bodyBytesSent = 0;
  this->_keepAlive = true;
  this->_cgiBytesSent = 0;
  this->_cgiResponse.clear();
  this->_streamOffset = 0;
  this->_isSending = false;
  this->_isFullySent = false;
}

void  HttpResponse::openAndSaveBodyFileStream(const std::string& path)
{
  this->_bodyFileStream.open(path.c_str(), std::ios::in | std::ios::binary);
  if (!this->_bodyFileStream)
  {
    logger(LOG_FATAL,
        "Warning: The file should be readable but an issue was detected.");
    return ;
  }
  logger(LOG_INFO, "File opened and filestream stored for response body: filename='"
      + path);
}

void  HttpResponse::closeBodyFileStream(const int& fd)
{
  if (this->_bodyFileStream.is_open())
  {
    logger(LOG_INFO,
      "Closed response body filestream for client fd=" + toString(fd));
    this->_bodyFileStream.close();
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
  else if (!this->_bodyFileStream) {
    // TODO we need to close the client here
  }
  else
  {
    if (this->_bufferOffset == this->_bufferSize)
    {
      this->_bufferOffset = 0;
      this->_bodyFileStream.seekg(this->_streamOffset);
      this->_bodyFileStream.read(&this->_bodyBuffer[0], READ_CHUNK_SIZE);
      std::streamsize bytesRead = this->_bodyFileStream.gcount();
      ssize_t bytesSent = send(fd, this->_bodyBuffer, bytesRead, 0);
      if (bytesSent > 0)
      {
        this->_bodyBytesSent += bytesSent;
        this->_bufferOffset += bytesSent;
        this->_bufferSize = bytesRead;
      }
      else if (bytesSent == 0)
        logger(LOG_DEBUG, "bytesSent == 0");
      else if (bytesSent == -1)
        logger(LOG_DEBUG, "bytesSent == -1");
      
      this->_streamOffset += bytesRead;
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
    return (true);
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

/*
void HttpResponse::setBodyFileFd(const int &fd)
{
  this->_bodyFileFd = fd;
}
*/

std::string &HttpResponse::getBodyFilePath(void)
{
  return (this->_bodyFilePath);
}

/*
int &HttpResponse::getBodyFileFd(void)
{
  return (this->_bodyFileFd);
}
*/

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

void  HttpResponse::addExtraHeader(const std::string& connectionHeader,
    const std::string& version)
{
  size_t  pos = this->_cgiResponse.find("\r\n\r\n");
  if (pos == std::string::npos)
    logger(LOG_WARNING, "Unable to add the connection header");
  std::string headerPart = this->_cgiResponse.substr(0, pos);
  std::vector<std::string> headersVec;
  std::istringstream iss(headerPart);
  std::string line;
  while (std::getline(iss, line))
  {
    std::istringstream keyVal(line);
    std::string key;
    keyVal >> key;
    headersVec.push_back(toUpper(key));
  }
  std::string extraHeader;
  bool  needHttpVersion = false;
  std::vector<std::string>::iterator it = headersVec.begin();
  for (; it != headersVec.end(); it++)
  {
    if (it->find("HTTP/") != std::string::npos)
      break;
  }
  if (it == headersVec.end())
  {
    extraHeader += version + " 200 OK\r\n";
    needHttpVersion = true;
  }
  if (std::find(headersVec.begin(), headersVec.end(), "CONTENT-LENGTH:") == headersVec.end())
    extraHeader += "Content-Length: " + toString(this->_cgiResponse.size() - pos - 4) + "\r\n";
  if (std::find(headersVec.begin(), headersVec.end(), "CONNECTION:") == headersVec.end())
    extraHeader += connectionHeader.substr(0, connectionHeader.size() - 2);

  if (needHttpVersion)
    this->_cgiResponse.insert(0, extraHeader);
  else
    this->_cgiResponse.insert(pos + 2, extraHeader);
  logger(LOG_DEBUG, "cgi response:\n" + this->_cgiResponse + "\n");
}
