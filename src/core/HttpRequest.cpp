/* ***********************************else *************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:30:00 by srandria          #+#    #+#             */
/*   Updated: 2025/08/15 10:13:54 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/HttpRequest.hpp"
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>

HttpRequest::HttpRequest(void) :_isComplete(false),  _bodyBytesRead(0), _contentLength(0),
  _isChunked(false)
{

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
  logger(LOG_INFO, "End of header detected (\\r\\n\\r\\n)");
  this->parseHeader_(raw_request, pos);
}

// TODO Need code formating
void  HttpRequest::parseHeader_(const std::string &raw_request,
    const size_t endOfHeader)
{
  logger(LOG_INFO, "Parsing of header begins.");
  std::istringstream iss(raw_request);
  iss >> this->_method;
  iss >> this->_path;
  iss >> this->_version;
  logger(LOG_INFO, "Method -> " + this->_method);
  logger(LOG_INFO, "path -> " + this->_path);
  logger(LOG_INFO, "version -> " + this->_version);
  std::string line;

  std::getline(iss, line);
  if (!this->isValid())
  {
    this->_isComplete = true;
    return ;
  }
  while (std::getline(iss, line) && line != "\r")
  {
    size_t      pos = line.find(":");
    std::string key;
    std::string value;

    key = line.substr(0, pos);
    if (pos == std::string::npos)
      break ;
    value = line.substr(pos + 2);
    value.erase(value.size() - 1);
    this->_headers[toUpper(key)] = toUpper(value);
  }
  this->setIsChunckedValue();
  if (this->_method == "POST")
  {
    this->_contentLength = std::atoi(this->_headers["CONTENT-LENGTH"].c_str());
    logger(LOG_INFO, "Saved content length = " + this->_headers["CONTENT-LENGTH"]);
    logger(LOG_INFO, "POST detected here");
    std::string bodyPart;

    bodyPart = raw_request.substr(endOfHeader + std::string ("\r\n\r\n").size());
    // TODO test it with bodyPart.size() == this->_contentLength
    // and try remove this->_bodyBytesRead = this->_contentLength with it
    if (bodyPart.size() >= this->_contentLength && !this->isChunked())
    {
      logger(LOG_DEBUG, "=================== Detect it ==================");
      bodyPart.resize(this->_contentLength);
      this->_bodyBytesRead = this->_contentLength;
      logger(LOG_DEBUG, "set to true lev 1");
      this->_isComplete = true;
    }
    else
      this->_bodyBytesRead = bodyPart.size();
    this->_body.append(bodyPart);
    logger(LOG_INFO, "body saved -> [" + this->_body + "]");
    std::ostringstream oss;
    oss << "_bodyBytesRead value here -> " << this->_bodyBytesRead << std::endl;
      logger(LOG_INFO, oss.str());
  }
  else
  {
    logger(LOG_DEBUG, "set to true lev 2");
    this->_isComplete = true;
  }
}

void  HttpRequest::setIsChunckedValue(void)
{
  logger(LOG_DEBUG, "in function setIsChunckedValue");
  std::map<std::string, std::string>::const_iterator it = this->getHeaders().find("transfer-encoding");
  if (it != this->getHeaders().end())
  {
    logger(LOG_DEBUG, "transfer-encoding header found");
    if (it->second == "chunked")
    {
      this->_isChunked = true;
      logger(LOG_DEBUG, "isChunked set to true");
      this->_contentLength = -1;
      return ;
    }
  }
  logger(LOG_DEBUG, "isChunked set to false");
}

bool  HttpRequest::isChunked()
{
  return (this->_isChunked);
}

bool  HttpRequest::isValid(void) const
{
  if (this->_method != "GET" && this->_method != "POST" && this->_method != "DELETE")
  {
    logger(LOG_ERROR, "INVALID method");
    return (false);
  }
  logger(LOG_INFO, "valid method");
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

void  HttpRequest::appendToBody(std::string str)
{
  if (this->isChunked())
  {
    logger(LOG_DEBUG, "appendToBody for chunked");
    logger(LOG_DEBUG, "infinit loop in HttpRequest::appendToBody");
    while (1)
      ;
  }
  else
  {
    logger(LOG_DEBUG, "appendToBody for not chunked");
    if (str.size() + this->_bodyBytesRead > this->_contentLength)
    {
      str.resize(this->_contentLength - this->_bodyBytesRead);
      this->_bodyBytesRead = this->_contentLength;
    }
    else
      this->_bodyBytesRead += str.size();
    this->_body.append(str);
    if (this->_bodyBytesRead == this->_contentLength)
    {
      this->_isComplete = true;
      this->parseBody();
    }
  }
}

const std::string& HttpRequest::getBody(void) const
{
  return (this->_body);
}

const std::string& HttpRequest::getPath(void) const
{
  return (_path);
}

const std::map<std::string, std::string>& HttpRequest::getHeaders(void) const
{
  return (_headers);
}

void HttpRequest::shiftBufferAfterRequest()
{
  logger(LOG_DEBUG, "in shiftBufferAfterRequest");
  this->_method.clear();
  this->_path.clear();
  this->_headers.clear();
  this->_body.clear();
  this->_version.clear();
  this->_isComplete = false;
  this->_bodyBytesRead = 0;
  this->_contentLength = 0;
  this->_isChunked = false;
}

bool  HttpRequest::isBodySizeAllowed(void)
{
  logger(LOG_DEBUG, "In function isBodySizeAllowed");
  std::ostringstream oss;

  LocationConfig  location = this->getMatchingLocation_(this->_path, this->getServerConf());
  oss << "Content-Length [" << this->_contentLength << "]\n" <<
    "client_max_body_size [" << location.client_max_body_size << "]";
  logger(LOG_DEBUG, oss.str());
  
  if (this->_contentLength > location.client_max_body_size)
  {
    logger(LOG_DEBUG, "Too large");
    return (false);
  }
  logger(LOG_DEBUG, "Body size is allowed.");
  return (true);
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

