/* ***********************************else *************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:30:00 by srandria          #+#    #+#             */
/*   Updated: 2025/08/21 11:30:28 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/HttpRequest.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/types.h>

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
  std::string headerPart = raw_request.substr(0, endOfHeader);
  std::istringstream iss(headerPart);
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
  std::string bodyPart;
  if (this->_method == "POST")
  {
    logger(LOG_DEBUG, "@@@@@@@@@@@@ BOdy part values");
    bodyPart = raw_request.substr(endOfHeader + std::string ("\r\n\r\n").size());
    std::cout.write(bodyPart.c_str(), bodyPart.size());
  }
  if (this->_method != "POST")
    this->_isComplete = true;
  else if (this->isChunked())
    this->extractBodyFromResponse(bodyPart);
  else
  {
    this->_contentLength = std::atoi(this->_headers["CONTENT-LENGTH"].c_str());
    // TODO test it with bodyPart.size() == this->_contentLength
    // and try remove this->_bodyBytesRead = this->_contentLength with it
    if (bodyPart.size() >= this->_contentLength && !this->isChunked())
    {
      bodyPart.resize(this->_contentLength);
      this->_bodyBytesRead = this->_contentLength;
      this->_isComplete = true;
    }
    else
      this->_bodyBytesRead = bodyPart.size();
    this->_body.append(bodyPart);
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
    //logger(LOG_DEBUG, "infinit loop in HttpRequest::appendToBody");
    this->extractBodyFromResponse(str);
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
    this->_body.append(str.c_str(), str.size());
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
  this->_bodyBuffChunked.clear();
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

void HttpRequest::extractBodyFromResponse(const std::string& bodyPart)
{
    logger(LOG_DEBUG, "in function extractBodyFromResponse");
    size_t contentSize = 0;

    this->_bodyBuffChunked.append(bodyPart.c_str(), bodyPart.size());
    


    static int  lock = 0;
    while (this->isNextChunkReady(contentSize))
    {
        //while (1);
        lock++;
        logger(LOG_DEBUG, " &&&&&&&&&&&&&&&&&&&&&& this->_bodyBuffChunked values");
        //std::cout.write(bodyPart.c_str(), bodyPart.size());
        std::cout.write(this->_bodyBuffChunked.c_str(), this->_bodyBuffChunked.size());
        logger(LOG_DEBUG, "&&&&&&&&&&&&&&&&&&&&&&&&&&&&");

        /*
        if (lock == 2)
          while (1);
        if (this->hasError())
        {
          logger(LOG_ERROR, "Error Detected");
          while (1);
        }
        */

        logger(LOG_DEBUG, "Found");

        size_t pos = this->_bodyBuffChunked.find("\r\n");
        size_t contentStart = pos + 2;
        if (contentSize == 0)
        {
            logger(LOG_DEBUG, "last chunk received (0 length)");
            this->_isComplete = true;
            // Supprimer correctement "0\r\n\r\n"
            this->_bodyBuffChunked.erase(0, pos + 4); 
            break; // sortir de la boucle
        }
        else
        {
            logger(LOG_DEBUG, "the preview contentSize [" + toString(contentSize) + "]");
            //while (1)
            //  ;
            this->_body.append(this->_bodyBuffChunked.c_str(), contentStart, contentSize);
            logger(LOG_DEBUG, "chunk appended, total body size = " + toString(this->_body.size()));

            // Supprime la taille + CRLF + données + CRLF
            this->_bodyBuffChunked.erase(0, contentStart + contentSize + 2);
        }
    }
}


bool  HttpRequest::isNextChunkReady(size_t& contentSize)
{
  logger(LOG_DEBUG, "In function isNextChunkReady");
  size_t pos = this->_bodyBuffChunked.find("\r\n");
  contentSize = 0;
  // for test

  size_t posbr = this->_bodyBuffChunked.find("\r");
  size_t posbn = this->_bodyBuffChunked.find("\n");
  if (posbr != std::string::npos)
    logger(LOG_DEBUG, "Position of \\r [" + toString(posbr) + "]");
  if (posbn != std::string::npos)
    logger(LOG_DEBUG, "Position of \\n [" + toString(posbn) + "]");

  

  ////

  if (pos == std::string::npos)
  {
    logger(LOG_DEBUG, "aucun \\r\\n trouve");
    return (false);
  }

  std::string hexaSize = this->_bodyBuffChunked.substr(0, pos);
  logger(LOG_DEBUG, "hexa value [" + hexaSize + "]");
  long value = strtol(this->_bodyBuffChunked.substr(0, pos).c_str(),
        0, 16);
  std::cout.write(this->_bodyBuffChunked.c_str(), this->_bodyBuffChunked.size());
  logger(LOG_DEBUG, "NOus allons parcourir les [" + toString(this->_bodyBuffChunked.size()) + "caracteres");
  logger(LOG_DEBUG, "decimal value from hexa [" + toString(value) + "]");
  /*
  if (value == 0)
  {
    std::string lengthStr= this->_bodyBuffChunked.substr(0, pos).c_str();
    logger(LOG_DEBUG, "lengthStr.size() -> [" + toString(lengthStr.size()) + "]");
    logger(LOG_DEBUG, "lengthStr -> [" + lengthStr + "]");
  }
  */
  if ( value < 0)
  {
    this->setError();
    logger(LOG_DEBUG, "ON a un probleme");
    return false;
  }
  contentSize = static_cast<size_t>(value);
  logger(LOG_DEBUG, "_bodyBuffChunked.size() -> " + toString(this->_bodyBuffChunked.size()));
  if (this->_bodyBuffChunked.size() >= pos + 2 + contentSize + 2)
  {
    logger(LOG_DEBUG, "Yes, Next Chunk is Ready ********************************************************************");
    return (true);
  }
  else
  {
    logger(LOG_DEBUG, "No, Next Chunk is not Ready");
    return (false);
  }
}

/*
void HttpRequest::extractBodyFromResponse(const std::string& bodyPart)
{
    logger(LOG_DEBUG, "in function extractBodyFromResponse");
    size_t contentSize = 0;

    // On accumule les données reçues
    this->_bodyBuffChunked.append(bodyPart);

    while (this->isNextChunkReady(contentSize))
    {
        if (this->_hasError)
        {
            logger(LOG_DEBUG, "error while parsing chunk");
            return;
        }

        size_t pos = this->_bodyBuffChunked.find("\r\n");
        if (pos == std::string::npos)
        {
            logger(LOG_DEBUG, "Unexpected: no CRLF found");
            this->setError();
            return;
        }

        size_t contentStart = pos + 2;

        // Dernier chunk : "0\r\n\r\n"
        if (contentSize == 0)
        {
            logger(LOG_DEBUG, "last chunk received (0 length)");
            this->_isComplete = true;
            // Supprimer le dernier chunk et CRLF final
            if (_bodyBuffChunked.size() >= pos + 4)
                this->_bodyBuffChunked.erase(0, pos + 4);
            else
                this->_bodyBuffChunked.clear();
            break;
        }

        // Copier le chunk dans le corps (sécurisé pour binaire)
        this->_body.append(
            this->_bodyBuffChunked.data() + contentStart,
            contentSize
        );
        logger(LOG_DEBUG, "chunk appended, total body size = " + toString(this->_body.size()));

        // Supprimer la taille + CRLF + données + CRLF
        if (_bodyBuffChunked.size() >= contentStart + contentSize + 2)
            this->_bodyBuffChunked.erase(0, contentStart + contentSize + 2);
        else
        {
            logger(LOG_DEBUG, "Partial chunk received, wait for more data");
            break; // on attend le reste du chunk
        }
    }
}


bool HttpRequest::isNextChunkReady(size_t& contentSize)
{
    logger(LOG_DEBUG, "In function isNextChunkReady");
    size_t pos = this->_bodyBuffChunked.find("\r\n");
    contentSize = 0;

    if (pos == std::string::npos)
    {
        logger(LOG_DEBUG, "aucun \\r\\n trouve");
        return false;
    }

    std::string hexaSize = this->_bodyBuffChunked.substr(0, pos);
    long value = strtol(hexaSize.c_str(), 0, 16);

    if (value < 0)
    {
        logger(LOG_DEBUG, "Invalid chunk size");
        this->setError();
        return false;
    }

    contentSize = static_cast<size_t>(value);

    // Vérifie si on a assez de données pour ce chunk + CRLF final
    if (_bodyBuffChunked.size() >= pos + 2 + contentSize + 2)
    {
        logger(LOG_DEBUG, "Next chunk is ready, size = " + toString(contentSize));
        return true;
    }

    logger(LOG_DEBUG, "Next chunk not ready yet");
    return false;
}
*/

void  HttpRequest::setError(void)
{
  this->_hasError = true;
}

bool  HttpRequest::hasError(void)
{
  return (this->_hasError);
}

