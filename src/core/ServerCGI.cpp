/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerCGI.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zramahaz <zramahaz@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/21 13:24:59 by zramahaz          #+#    #+#             */
/*   Updated: 2025/08/21 17:16:06 by zramahaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/Server.hpp"
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iterator>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <cstring> // pour strdup

void Server::handleCgiGetRequest_(const int fd)
{
  std::string absolutePath = this->getCurrentLocation().root  + "/" \
    + getUriPath_(fd).substr(this->getCurrentLocation().path.size());
  logger(LOG_DEBUG, "preparing CGI handler");
  logger(LOG_DEBUG, "getRequestUri_ -> " + this->getRequestUri_(fd));
  logger(LOG_DEBUG, "getUriPath_    -> " + this->getUriPath_(fd));
  logger(LOG_DEBUG, "absolutePath   -> " + absolutePath);

  // error cgi_path
  if (!this->isFile_(this->getCurrentLocation().cgi_path) \
      && !this->isExecutable_(this->getCurrentLocation().cgi_path))
  {
    // TODO : retourne une page d'erreur avec status = 500
    // "500 Internal Server Error: CGI interpreter not available
    return ;
  }
  // build env variables
  char **envp = this->buildEnvpForExecve_(fd);

  // execute the script and cummunicate with him
  int cgi_pipe[2];
  if (pipe(cgi_pipe) == -1)
  {
    logger(LOG_ERROR, "Error with function pipe()");
    return ;
  }
  this->_clients[fd]->getRequest()._isCgiRequest = true;
  int pid = fork();
  if (pid < 0)
  {
    // TODO Need to do something here for HTTP response
    logger(LOG_FATAL, "fork failed");
    return ;
  }
  else if (pid == 0)
  {
    char *argv[] = {
        (char*)this->getCurrentLocation().cgi_path.c_str(),
        (char*)absolutePath.c_str(),
        NULL
    };
    close(cgi_pipe[0]);
    dup2(cgi_pipe[1], STDOUT_FILENO);
    close(cgi_pipe[1]);
    logger(LOG_DEBUG,
        "🚀 Executing CGI handler [" + this->getFileName(this->getUriPath_(fd)) + "]");
    execve(this->getCurrentLocation().cgi_path.c_str(), argv, envp);
    exit(0);
  }
  else
  {
    logger(LOG_DEBUG, "I am parent");
    this->_pipeFdReadComplete = false;
    this->_pipeFd.push_back(cgi_pipe[0]);
    this->_pipeFdClient[cgi_pipe[0]] = fd;
    this->setNonBlocking_(cgi_pipe[0]);
    this->addFdToPoll_(cgi_pipe[0]);
    close(cgi_pipe[1]);
  }
}

char  **Server::buildEnvpForExecve_(const int fd)
{
  std::map<std::string, std::string>  envMap;
  std::ostringstream  oss;

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
  oss << "        " << "SERVER_PROTOCOL = " << envMap["SERVER_PROTOCOL"] << std::endl
    << std::endl;
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

void Server::handleCgiPostRequest_(const int fd)
{
  logger(LOG_DEBUG, "in handleCgiPostRequest_");
  (void)fd;
}

void  Server::handleCgiRead(const int& pipeFd, const int& clientFd)
{
  // a modifier, C'est juste pour voir tous les chaine lus
  if (this->_clients[clientFd]->_isReadingCgiResponse == false)
  {
    logger(LOG_DEBUG,
        "📥 Receiving CGI-generated HTTP response in parent process ...");
    this->_clients[clientFd]->_isReadingCgiResponse = true;
  }
  static std::string fullStr;
  char buffer[12];
  int count = read(pipeFd, buffer, sizeof(buffer));
  if (count == -1)
  {
    logger(LOG_ERROR, "read failed");
  } 
  else if (count == 0)
  {
    logger(LOG_INFO, "Parent received CGI response, ready to send.");
    this->_pipeFdReadComplete = true;
    this->_clients[clientFd]->_isReadingCgiResponse = false;
    this->unregisterCgiFd(pipeFd);
    this->setPollOut_(clientFd);
    std::cout << fullStr << std::endl;
  }
  else if (count > 0)
  {
    buffer[count] = 0;
    fullStr.append(buffer);
  }
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
