/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerCGI.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zramahaz <zramahaz@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/21 13:24:59 by zramahaz          #+#    #+#             */
/*   Updated: 2025/08/25 10:34:55 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/Server.hpp"
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <cstring> // pour strdup

void  Server::prepareAndLaunchCGI(const int& fd)
{
  LocationConfig  location = this->getCurrentLocation();
  std::string     localPath;
  localPath = location.root + '/' + getUriPath_(fd).substr(location.path.size());
  std::string cgiPath = location.cgi_path;
  if (this->isExecutable_(localPath))
  {
    if (!this->isFile_(cgiPath) || !this->isExecutable_(cgiPath))
      this->respondInternalServerError(fd);
    else
      this->launchCgiProcess(fd, localPath);
  }
  else
    this->responsNotExecutable(fd);
}

void  Server::respondInternalServerError(const int&fd)
{
  logger(LOG_DEBUG, "in function respondInternalServerError");
  /*
  HTTP/1.1 500 Internal Server Error
  Content-Type: text/html
  Content-Length: 64

  CGI script not available or not executable.
  */
  std::string body;
  std::string contentLength;
  std::string contentType = CT_TEXT;

  this->setStatus(500, fd);
  if (this->hasCustomErrorPage(500, fd))
    this->saveErrorBodyFilePath(500, fd, contentType, contentLength);
  else
  {
    body = "CGI script not available or not executable.";
    contentLength = toString(body.size());
  }

  std::ostringstream headers;

  headers << this->getVersion(fd) << " 500 Internal Server Error\r\n"
    << CT << " " << contentType << "\r\n"
    << CL << " " << contentLength << "\r\n"
    << this->buildConnectionHeader(fd);

  HttpResponse& response = this->_clients[fd]->getResponse();
  response.setHeader(headers.str());
  response.setBody(body);
}

void  Server::launchCgiProcess(const int& fd, const std::string& localPath)
{
  CgiPipes cgiPipes;
  const std::string method = this->getMethod(fd);

  if (pipe(cgiPipes.out_pipe) == -1 || (method == "POST" && pipe(cgiPipes.in_pipe)))
  {
    logger(LOG_ERROR, "Error with function pipe()");
    return ;
  }
  this->_clients[fd]->getRequest()._isCgiRequest = true;
  logger(LOG_DEBUG,
      "🚀 Executing CGI handler [" + this->getFileName(this->getUriPath_(fd)) + "] ...");
  int pid = fork();
  if (pid < 0)
  {
    // TODO Need to do something here for HTTP response
    logger(LOG_FATAL, "fork failed");
    return ;
  }
  else if (pid == 0)
    this->handleChildProcess(fd, localPath, cgiPipes);
  else
    this->handleParentProcess(fd, cgiPipes);
}

void  Server::handleChildProcess(const int&fd, const std::string& localPath,
    const CgiPipes& cgiPipes)
{
  char **envp = this->buildEnvpForExecve_(fd);

  char *argv[] = {
      (char*)this->getCurrentLocation().cgi_path.c_str(),
      (char*)localPath.c_str(),
      NULL
  };
  dup2(cgiPipes.out_pipe[1], STDOUT_FILENO);
  close(cgiPipes.out_pipe[0]);
  close(cgiPipes.out_pipe[1]);
  if (this->getMethod(fd) == "POST")
  {
    dup2(cgiPipes.in_pipe[0], STDIN_FILENO);
    close(cgiPipes.in_pipe[0]);
    close(cgiPipes.in_pipe[1]);
  }
  execve(this->getCurrentLocation().cgi_path.c_str(), argv, envp);
  perror("execve failed");
  exit(0);
}

void  Server::handleParentProcess(const int&fd, const CgiPipes& cgiPipes)
{
  this->_pipeFdReadComplete = false;
  this->_pipeFd.push_back(cgiPipes.out_pipe[0]);
  this->_pipeFdClient[cgiPipes.out_pipe[0]] = fd;
  this->setNonBlocking_(cgiPipes.out_pipe[0]);
  this->addFdToPoll_(cgiPipes.out_pipe[0]);
  close(cgiPipes.out_pipe[1]);
  if (this->getMethod(fd) == "POST")
    close(cgiPipes.in_pipe[0]);
}

bool  Server::isCGIRequest(const int&fd)
{
  LocationConfig  location = this->getCurrentLocation();
  std::string     localPath;
  localPath = location.root + '/' + getUriPath_(fd).substr(location.path.size());

  return (this->getFileExtension_(getUriPath_(fd)) ==
      this->getCurrentLocation().cgi_extension
      && !this->getCurrentLocation().cgi_extension.empty()
      && !this->getCurrentLocation().cgi_path.empty());
}

char  **Server::buildEnvpForExecve_(const int fd)
{
  std::map<std::string, std::string>  envMap;
  std::ostringstream  oss;

  envMap["VERSION"] = this->getVersion(fd);
  oss << "        " << "VERSION = " << envMap["VERSION"] << std::endl;;
  envMap["REQUEST_METHOD"] = this->getMethod(fd);
  oss << "        " << "REQUEST_METHOD = " << envMap["REQUEST_METHOD"] << std::endl;;
  size_t pos = this->getRequestUri_(fd).rfind("?");
  if (pos != std::string::npos)
  {
    std::string queryString = this->getRequestUri_(fd).substr(pos + 1);
    envMap["QUERY_STRING"] = queryString;
    oss << "        " << "QUERY_STRING = " << envMap["QUERY_STRING"] << std::endl;;
  }
  envMap["SCRIPT_NAME"] = this->getUriPath_(fd);
  oss << "        " << "SCRIPT_NAME = " << envMap["SCRIPT_NAME"] << std::endl;
  envMap["SERVER_PROTOCOL"] = this->getVersion(fd);
  oss << "        " << "SERVER_PROTOCOL = " << envMap["SERVER_PROTOCOL"] << std::endl;
  envMap["CONNECTION"] = this->buildConnectionHeader(fd);
  oss << "        " << "CONNECTION = " << envMap["CONNECTION"] << std::endl;;
  logger(LOG_DEBUG, "⚙️  CGI environment variables set for child process\n" + oss.str());
  std::vector<std::string>  envVars;
  for (std::map<std::string, std::string>::iterator it = envMap.begin();
      it != envMap.end(); it++)
    envVars.push_back(it->first + "=" + it->second);

  char **envp = new char*[envVars.size() + 1]();
  for (size_t i = 0; i < envVars.size(); i++)
  {
    envp[i] = strdup(envVars[i].c_str());
    if (envp[i] == NULL)
    {
      for (size_t j = 0; j < i; j++)
        free(envp[j]);
      delete [] envp;
      throw std::runtime_error("strdup fail");
    }
  }
  envp[envVars.size()] = NULL;
  return (envp);
}

void  Server::handleCgiRead(const int& pipeFd, const int& clientFd)
{
  Client* client = this->_clients[clientFd];
  if (client->_isReadingCgiResponse == false)
  {
    logger(LOG_INFO,
        "📥 Receiving CGI-generated HTTP response in parent process ...");
    client->_isReadingCgiResponse = true;
  }
  char buffer[8192];
  int count = read(pipeFd, buffer, sizeof(buffer));
  if (count == -1)
  {
    logger(LOG_ERROR, "CGI read failure, fallback response will be sent");
    this->_clients[pipeFd]->getRequest()._isCgiRequest = false;
    this->unregisterCgiFd(pipeFd);
    this->respondFallbackError(clientFd);
    this->saveHeaderAndBodySize(clientFd);
    this->setPollOut_(clientFd);
  } 
  else if (count == 0)
  {
    logger(LOG_INFO,
      "Parent received CGI response, ready to send to client fd=" + toString(clientFd));
    this->_pipeFdReadComplete = true;
    client->_isReadingCgiResponse = false;
    client->getResponse().saveCgiRespondSize(clientFd);
    this->unregisterCgiFd(pipeFd);
    this->setPollOut_(clientFd);
  }
  else if (count > 0)
    client->getResponse().appendCgiResponse(buffer, count);
}

void  Server::unregisterCgiFd(const int& pipeFd)
{
  std::vector<struct pollfd>::iterator itPollFd = this->_pool_fds.begin();
  close(pipeFd);
  while (itPollFd != this->_pool_fds.end())
  {
    if (itPollFd->fd == pipeFd)
    {
      itPollFd = this->_pool_fds.erase(itPollFd);
      logger(LOG_DEBUG,
        "🟢 Pipe fd=" + toString(pipeFd) + " successfully removed from poll monitoring");
      return ;
    }
    else
      ++itPollFd;
  }
  logger(LOG_DEBUG,
    "⚠️ Failed to remove pipe fd=" + toString(pipeFd) + ": not found in poll monitoring");
}
