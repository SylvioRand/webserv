/* ***********************************else *************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:30:00 by srandria          #+#    #+#             */
/*   Updated: 2025/08/05 09:08:03 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/HttpRequest.hpp"
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>

HttpRequest::HttpRequest(void) :_isComplete(false),  _bodyBytesRead(0), _contentLength(0)
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
  return ;
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
    this->_headers[key] = value;
  }
  if (this->_method == "POST")
  {
    this->_contentLength = std::atoi(this->_headers["Content-Length"].c_str());
    logger(LOG_INFO, "Saved content length = " + this->_headers["Content-Length"]);
    logger(LOG_INFO, "POST detected here");
    std::string bodyPart;

    bodyPart = raw_request.substr(endOfHeader + std::string ("\r\n\r\n").size());
    if (bodyPart.size() >= this->_contentLength)
    {
      bodyPart.resize(this->_contentLength);
      this->_bodyBytesRead = this->_contentLength;
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
    this->_isComplete = true;
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
  if (str.size() + this->_bodyBytesRead > this->_contentLength)
  {
    str.resize(this->_contentLength - this->_bodyBytesRead);
    this->_bodyBytesRead = this->_contentLength;
  }
  else
    this->_bodyBytesRead += str.size();
  this->_body.append(str);
  if (this->_bodyBytesRead == this->_contentLength)
    this->_isComplete = true;
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

