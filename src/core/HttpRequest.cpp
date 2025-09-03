/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:30:00 by srandria          #+#    #+#             */
/*   Updated: 2025/08/29 08:29:07 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/HttpRequest.hpp"
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/types.h>

HttpRequest::HttpRequest(void) : _isComplete(false),  _bodyBytesRead(0),
  _contentLength(0), _isChunked(false), _hasError(false), _cgiOffset(0),
  _hasContentLength(false), _hasBoundary(false), _isBodySizeAllowed(true),
  _isCgiRequest(false), _allFilesSaved(false), _isReadingRequest(false)
{
  //this->_body.reserve(9999999999);
}

// TODO verifie for leaks if we need to free something
HttpRequest::~HttpRequest(void)
{

}

void  HttpRequest::parse(const std::string &raw_request)
{
  size_t pos = raw_request.find("\r\n\r\n");

  if (pos == std::string::npos)
    return ;
  logger(LOG_DEBUG, "End of header detected (\\r\\n\\r\\n)");
  std::ostringstream oss;
  oss << "👇👇👇👇👇 Headers from request 👇👇👇👇👇" << std::endl << std::endl;
  oss.write(raw_request.c_str(), pos);
  oss << std::endl << std::endl;
  logger(LOG_DEBUG, oss.str());
  this->parseHeader_(raw_request, pos);
  }

void  HttpRequest::parseHeader_(const std::string &raw_request,
    const size_t endOfHeader)
{
  logger(LOG_DEBUG, "Parsing of header begins.");
  std::string headerPart = raw_request.substr(0, endOfHeader);
  std::istringstream iss(headerPart);
  iss >> this->_method;
  iss >> this->_path;
  iss >> this->_version;
  logger(LOG_DEBUG, "Method     -> " + this->_method);
  logger(LOG_DEBUG, "requestURI -> " + this->_path);
  logger(LOG_DEBUG, "version    -> " + this->_version);

  this->fillHeadersMap(iss);
  this->setIsChunckedValue();
  this->setHasContentLength();
  this->setHasBoundary();
  this->_location = this->getMatchingLocation_(this->_path,
      this->getServerConf());
  this->_client_max_body_size = this->_location.client_max_body_size;
  logger(LOG_INFO, "verif location -> " + this->_location.path);
  if (this->_hasContentLength && this->_contentLength > this->_client_max_body_size)
  {
    this->_isBodySizeAllowed = false;
    this->_allFilesSaved = true;
    this->markRequestComplete();
    return ;
  }
  if (this->_method == "POST")
  {
    std::string bodyPart;

    bodyPart = raw_request.substr(endOfHeader + std::string ("\r\n\r\n").size());
   
    this->extractRequestBody(bodyPart);
  }
  else
    this->markRequestComplete();
}

void  HttpRequest::extractRequestBody(std::string& bodyPart)
{
  if (this->_body.size() + bodyPart.size() > this->_location.client_max_body_size ||
      (this->_hasContentLength && this->_contentLength > this->_client_max_body_size))
  {
    this->_isBodySizeAllowed = false;
    this->markRequestComplete();
    return ;
  }
  else if (this->_isChunked)
  {
    this->handleChunkedEncoding(bodyPart);
  }
  else if (this->_hasBoundary)
  {
    this->handleMultipartFormData(bodyPart);
  }
  else if (this->_hasContentLength)
  {
    logger(LOG_FATAL, "yes headers has Content-Length");
    while (1) ;
    this->handleFixedLengthBody(bodyPart);
  }
}

void HttpRequest::extractRequestBody(const char *data, size_t len)
{
  if ((this->_isBodySizeAllowed && this->_body.size() + len > this->_location.client_max_body_size) ||
      (this->_hasContentLength && this->_contentLength > this->_client_max_body_size))
  {
    this->_isBodySizeAllowed = false;
    this->markRequestComplete();
    return ;
  }
  if (_isChunked)
      handleChunkedEncoding(data, len);
  else if (_hasBoundary)
      handleMultipartFormData(data, len);
  else if (_hasContentLength)
      handleFixedLengthBody(data, len);
}

void  HttpRequest::handleMultipartFormData(const std::string& bodyPart)
{
  //size_t  endOfBody = std::string::npos;

  this->_body.append(bodyPart.c_str(), bodyPart.size());
  std::string endBoundary;
  if (this->_body.size() >= this->_endBoundary.size())
    endBoundary = this->_body.substr(this->_body.size() - this->_endBoundary.size(),
        this->_endBoundary.size());
  //endOfBody = this->_body.find(this->_endBoundary);
  if (endBoundary != this->_endBoundary)
    return ;
  // TODO need to test the first case
  if (this->_isCgiRequest)
    this->markRequestComplete();
  else
    this->parseMultipartBody();
}

void  HttpRequest::handleMultipartFormData(const char* bodyPart, const size_t len)
{
  //size_t  endOfBody = std::string::npos;

  this->_body.append(bodyPart, len);
  std::string endBoundary;
  if (this->_body.size() >= this->_endBoundary.size())
    endBoundary = this->_body.substr(this->_body.size() - this->_endBoundary.size(),
        this->_endBoundary.size());

  //endOfBody = this->_body.find(this->_endBoundary);
  if (endBoundary != this->_endBoundary)
    return ;
  // TODO need to test the first case
  if (this->_isCgiRequest)
  {
    // TODO verify if we need to set POLLOUT
    //if (endOfBody != std::string::npos)
      this->markRequestComplete();
  }
  else
    this->parseMultipartBody();
}


void  HttpRequest::handleChunkedEncoding(const std::string& bodyPart)
{
  size_t contentSize = 0;

  this->_bodyBuffChunked.append(bodyPart.c_str(), bodyPart.size());
  while (this->isNextChunkReady(contentSize))
  {
    size_t pos = this->_bodyBuffChunked.find("\r\n");
    size_t contentStart = pos + 2;
        while (1);
    if (contentSize == 0)
    {
      logger(LOG_INFO,
        "[HTTP] Successfully received full request body (chunked transfer completed).");

      if (_hasBoundary)
        this->parseMultipartBody();
      this->markRequestComplete();
      break; // sortir de la boucle
    }
    else
    {
      this->_body.append(this->_bodyBuffChunked, contentStart, contentSize);
      this->_bodyBuffChunked.erase(0, contentStart + contentSize + 2);
    }
  }
}

void  HttpRequest::handleChunkedEncoding(const char *bodyPart, size_t len)
{
  size_t contentSize = 0;

  this->_bodyBuffChunked.append(bodyPart, len);
  while (this->isNextChunkReady(contentSize))
  {
    size_t pos = this->_bodyBuffChunked.find("\r\n");
    size_t contentStart = pos + 2;
        while (1);
    if (contentSize == 0)
    {
      logger(LOG_INFO,
        "[HTTP] Successfully received full request body (chunked transfer completed).");

      if (_hasBoundary)
        this->parseMultipartBody();
      this->markRequestComplete();
      break; // sortir de la boucle
    }
    else
    {
      this->_body.append(this->_bodyBuffChunked, contentStart, contentSize);
      this->_bodyBuffChunked.erase(0, contentStart + contentSize + 2);
    }
  }
}

void  HttpRequest::handleFixedLengthBody(std::string& bodyPart)
{
  if (bodyPart.size() >= this->_contentLength)
  {
    bodyPart.resize(this->_contentLength);
    this->_bodyBytesRead = this->_contentLength;
    this->markRequestComplete();
  }
  else
    this->_bodyBytesRead = bodyPart.size();

  this->_body.append(bodyPart);
}

void  HttpRequest::handleFixedLengthBody(const char *bodyPart, const size_t len)
{
  if (len >= this->_contentLength)
  {
    //bodyPart.resize(this->_contentLength);
    this->_bodyBytesRead = this->_contentLength;
    this->markRequestComplete();
  }
  else
    this->_bodyBytesRead = len;

  this->_body.append(bodyPart, len);
}


void  HttpRequest::parseMultipartBody()
{
  std::string boundary = this->_boundary.substr(0, this->_boundary.size() - 2);
  size_t  start = this->_body.find(boundary) + 2;
  start += this->_boundary.size();
  size_t  end;
  size_t  lastBoundaryPos = this->_body.size() - this->_endBoundary.size();
  while (1)
  {
    size_t step = start + this->_boundary.size();
    end = this->_body.find(boundary, step);
    this->addToMultipartStruct(start, end);
    if (end == lastBoundaryPos)
    {
      this->markRequestComplete();
      return;
    }
    start = end + this->_boundary.size() + 2;
  }
}

void   HttpRequest::addToMultipartStruct(size_t& start, size_t& end)
{
  MultipartPart part; 

  // TODO NEDD TO CREATE FUNCTION FOR FILLING name/filename/contentType from header inside boundary
  size_t headerEnd = this->_body.find("\r\n\r\n");
  std::string headerPart = this->_body.substr(start, headerEnd - start);
  std::istringstream iss(headerPart);
  std::string line;
  std::srand(static_cast<unsigned int>(std::time(NULL)));

  while (std::getline(iss, line) && line != "\r")
  {
    size_t      pos = line.find(":");
    std::string key;
    std::string value;

    key = toUpper(line.substr(0, pos));
    if (pos == std::string::npos)
      break ;
    value = line.substr(pos + 2);
    size_t  filenamePos = value.find("filename=\"");
    if (filenamePos != std::string::npos)
    {
      if (key.find("CONTENT-DISPOSITION") != std::string::npos && filenamePos != std::string::npos)
      {
        size_t start = filenamePos + std::string("filename=\"").size();
        size_t end = value.find('"', start);
        std::string filename = value.substr(filenamePos + std::string("filename=\"").size(), end - start);
        part.filename = unique_filename(filename);
      }
    }
  }
  part.name = "srandria";
  part.contentType = "";
  part.fullySaved = false;
  part.offset = 0;
  size_t pos = this->_body.find("\r\n\r\n", start) + 4;
  part.data = this->_body.substr(pos, end - pos - 2);
  this->_multiPart.push_back(part);
}

bool  HttpRequest::isChunked()
{
  return (this->_isChunked);
}

bool  HttpRequest::isValid(void) const
{
  if (this->_method != "GET" && this->_method != "POST" && this->_method != "DELETE")
  {
    logger(LOG_WARNING, "INVALID method");
    return (false);
  }
  logger(LOG_DEBUG, "✅ valid method");
  return (true);
}

const std::string& HttpRequest::getMethod(void) const
{
  return (_method);
}

const std::string& HttpRequest::getVersion(void) const
{
  return (_version);
}

bool  HttpRequest::isComplete(void) const
{
  return (this->_isComplete);
}

void  HttpRequest::appendToBody(std::string& str)
{
  if (this->isChunked())
        this->extractBodyFromResponse(str);
  else
  {
    // TODO To move to another specific function/ and maybe nedd new implementation
    logger(LOG_DEBUG, "appendToBody for not chunked");
    if (str.size() + this->_bodyBytesRead > this->_contentLength)
    {
      str.resize(this->_contentLength - this->_bodyBytesRead);
      this->_bodyBytesRead = this->_contentLength;
    }
    else
      this->_bodyBytesRead += str.size();
    this->_body.append(str.c_str(), str.size());
    if (this->_bodyBytesRead == this->_contentLength)
    {
      this->markRequestComplete();
      this->parseBody();
    }
  }
}

const std::string& HttpRequest::getBody(void) const
{
  return (this->_body);
}

const std::string& HttpRequest::getPath(void)
{
  return (_path);
}

const std::map<std::string, std::string>& HttpRequest::getHeaders(void) const
{
  return (_headers);
}

void HttpRequest::shiftBufferAfterRequest()
{
  this->_method.clear();
  this->_path.clear();
  this->_headers.clear();
  this->_body.clear();
  this->_version.clear();
  this->_isComplete = false;
  this->_bodyBytesRead = 0;
  this->_contentLength = 0;
  this->_isChunked = false;
  this->_isCgiRequest = false;
  this->_bodyBuffChunked.clear();
  this->_hasError = false;
  this->_cgiOffset = 0;
  this->_hasContentLength = false;
  this->_hasBoundary = false;
  this->_boundary.clear();
  this->_endBoundary.clear();
  this->_isBodySizeAllowed = true;
  this->_isCgiRequest = false;
  this->_multiPart.clear();
  this->_allFilesSaved = false;
  this->_isReadingRequest = false;
}

LocationConfig HttpRequest::getMatchingLocation_(const std::string& uri, const ServerConfigConstIterator& cfg)
{
  LocationConfig  best_match;
  size_t best_length = 0;

  std::map<std::string, LocationConfig> locations = cfg->locations;
  for (std::map<std::string, LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); it++)
  {
    const std::string& path = it->first;
    if (uri.substr(0, path.size()) == path && path.size() > best_length)
    {
      best_match = it->second;
      best_length = path.size();
    }
  }
  if (best_length == 0)
  {
    logger(LOG_DEBUG, "root location will be created and used");
    return (this->createAndReturnRootLocation_(cfg));
  }
  logger(LOG_DEBUG, "matching LocationConfig found, path [" + best_match.path + "]");
  return (best_match);
}

LocationConfig  HttpRequest::createAndReturnRootLocation_(const ServerConfigConstIterator& cfg)
{
  LocationConfig  rootLocation;

  for (std::vector<std::string>::const_iterator it = cfg->indexs.begin();
      it != cfg->indexs.end(); it++)
    rootLocation.indexs.push_back((*it));

  for (std::vector<std::string>::const_iterator it = cfg->methods.begin();
      it != cfg->methods.end(); it++)
    rootLocation.methods.push_back((*it));

  for (std::map<int, std::string>::const_iterator it = cfg->error_pages.begin();
      it != cfg->error_pages.end(); it++)
    rootLocation.error_pages[it->first] = it->second;

  rootLocation.autoindex = cfg->autoindex;
  rootLocation.client_max_body_size = cfg->client_max_body_size;
  rootLocation.upload_dir = cfg->upload_dir;
  rootLocation.path = "root";
  rootLocation.root = cfg->root;
  return (rootLocation);
}

void  HttpRequest::markRequestComplete(void)
{
  logger(LOG_INFO, "✅ Request fully received");
  this->_isComplete = true;
}

void  HttpRequest::setServerConf(ServerConfigConstIterator serverConf)
{
  this->_serverConf = serverConf;
}

HttpRequest::ServerConfigConstIterator  HttpRequest::getServerConf(void)
{
  return (this->_serverConf);
}

void  HttpRequest::parseBody()
{

}

void HttpRequest::extractBodyFromResponse(const std::string& bodyPart)
{
  size_t contentSize = 0;

  this->_bodyBuffChunked.append(bodyPart.c_str(), bodyPart.size());
  while (this->isNextChunkReady(contentSize))
  {
    size_t pos = this->_bodyBuffChunked.find("\r\n");
    size_t contentStart = pos + 2;
    if (contentSize == 0)
    {
      logger(LOG_INFO,
        "[HTTP] Successfully received full request body (chunked transfer completed).");
      this->_bodyBuffChunked.erase(0, pos + 4); 
      this->markRequestComplete();
      break; // sortir de la boucle
    }
    else
    {
      this->_body.append(this->_bodyBuffChunked, contentStart, contentSize);
      this->_bodyBuffChunked.erase(0, contentStart + contentSize + 2);
    }
  }
}

bool  HttpRequest::isNextChunkReady(size_t& contentSize)
{
  size_t pos = this->_bodyBuffChunked.find("\r\n");
  contentSize = 0;
  if (pos == std::string::npos)
    return (false);

  std::string hexaSize = this->_bodyBuffChunked.substr(0, pos);
  long value = strtol(this->_bodyBuffChunked.substr(0, pos).c_str(),
        0, 16);
  if ( value < 0)
  {
    this->setError();
    logger(LOG_ERROR, "Error detected in function isNextChunkReady");
    return false;
  }
  contentSize = static_cast<size_t>(value);
  if (this->_bodyBuffChunked.size() >= pos + 2 + contentSize + 2)
    return (true);
  else
    return (false);
}

void  HttpRequest::setError(void)
{
  this->_hasError = true;
}

bool  HttpRequest::hasError(void)
{
  return (this->_hasError);
}


size_t  HttpRequest::getCgiOffset(void)
{
  return (this->_cgiOffset);
}

// TODO need test
void  HttpRequest::sendRequestBodyToCgi(const int&pipeFd, const int& clientFd)
{
  size_t bytes;
  if (this->getBody().size() > this->getCgiOffset())
  {
    bytes = write(pipeFd, this->_body.c_str() + this->_cgiOffset, this->_body.size() - this->_cgiOffset);
    if (bytes > 0)
      this->_cgiOffset += bytes;
  }
  if (this->_body.size() == this->_cgiOffset)
  {
    close(pipeFd);
    logger(LOG_INFO, "📤 Entire request body successfully written to CGI pipe (fd=" 
                 + toString(pipeFd) + ") for client fd=" 
                 + toString(clientFd));
  }
}

void  HttpRequest::setIsChunckedValue(void)
{
  std::map<std::string, std::string>::const_iterator it =
    this->getHeaders().find("TRANSFER-ENCODING");
  if (it != this->getHeaders().end())
  {
    if (it->second == "CHUNKED")
    {
      this->_isChunked = true;
      logger(LOG_DEBUG, "Chunked request detected");
      this->_contentLength = -1; // A voir si on en a vraiment besion
      return ;
    }
  }
}

void  HttpRequest::setHasContentLength(void)
{
  std::map<std::string, std::string>::iterator it =
    this->_headers.find("CONTENT-LENGTH");
  if (it != this->_headers.end())
  {
    this->_hasContentLength = true;
    this->_contentLength = std::atoi(it->second.c_str());
    logger(LOG_DEBUG, "Content-Length is detected in headers");
  }
}

void  HttpRequest::setHasBoundary(void)
{
  std::map<std::string, std::string>::iterator it =
    this->_headers.find("CONTENT-TYPE");
  if (it != this->_headers.end())
  {
    size_t pos = it->second.find("BOUNDARY");

    if (pos != std::string::npos)
      this->_hasBoundary = true;
  }
}

bool  HttpRequest::hasContentLength(void)
{
  return (_hasContentLength);
}

void  HttpRequest::fillHeadersMap(std::istringstream& iss)
{
  std::string line;
  std::getline(iss, line);
  if (!this->isValid())
  {
    this->markRequestComplete();
    return ;
  }
  while (std::getline(iss, line) && line != "\r")
  {
    size_t      pos = line.find(":");
    std::string key;
    std::string value;

    key = toUpper(line.substr(0, pos));
    if (pos == std::string::npos)
      break ;
    value = line.substr(pos + 2);
    value.erase(value.size());
    size_t  posBoundary = value.find("boundary=");
    if (key.find("CONTENT-TYPE") != std::string::npos && posBoundary != std::string::npos)
    {
      std::string boundary = "--" + value.substr(posBoundary + 9, value.size() - posBoundary + 9);
      while (!boundary.empty() &&
          (boundary[boundary.size() - 1] == '\r' || boundary[boundary.size() - 1] == '\n'))
        boundary.erase(boundary.size() - 1);
      this->_boundary = boundary;
      this->_endBoundary = boundary + "--\r\n";
    }
    this->_headers[toUpper(key)] = toUpper(value);
  }
}


bool  HttpRequest::hasBoundary_(void)
{
  return (this->_hasBoundary);
}

std::vector<MultipartPart>&  HttpRequest::getMultipart(void)
{
  return (this->_multiPart);
}

void  HttpRequest::setClientMaxBodySize(const size_t& size)
{
  this->_client_max_body_size = size;
}

LocationConfig HttpRequest::getLocation(void)
{
  return (this->_location);
}

bool  HttpRequest::isBodySizeAllowed(void)
{
  return (this->_isBodySizeAllowed);
}
