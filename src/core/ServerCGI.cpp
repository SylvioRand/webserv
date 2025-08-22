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
#include <iterator>
#include <map>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <cstring> // pour strdup

void Server::handleCgiGetRequest_(const int fd)
{
  std::string absolutePath = this->getCurrentLocation().root  + "/" \
    + getUriPath_(fd).substr(this->getCurrentLocation().path.size());
  logger(LOG_INFO, "in handleCgiGetRequest_");
  logger(LOG_INFO, "getRequestUri_-> " + this->getRequestUri_(fd));
  logger(LOG_INFO, "getUriPath_-> " + this->getUriPath_(fd));
  logger(LOG_INFO, "absolutePath-> " + absolutePath);

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
  int pid = fork();
  if (pid < 0)
  {
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

  envMap["REQUEST_METHOD"] = this->getMethod(fd);
  logger(LOG_ERROR, "REQUEST_METHOD = " + envMap["REQUEST_METHOD"]);
  size_t pos = this->getRequestUri_(fd).rfind("?");
  if (pos != std::string::npos)
  {
    std::string queryString = this->getRequestUri_(fd).substr(pos + 1);
    envMap["QUERY_STRING"] = queryString;
    logger(LOG_ERROR, "QUERY_STRING = " + envMap["QUERY_STRING"]);
  }
  envMap["SCRIPT_NAME"] = this->getUriPath_(fd);
  logger(LOG_ERROR, "SCRIPT_NAME = " + envMap["SCRIPT_NAME"]);
  envMap["SERVER_PROTOCOL"] = this->getVersion(fd);
  logger(LOG_ERROR, "SERVER_PROTOCOL = " + envMap["SERVER_PROTOCOL"]);

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

void  Server::handleCgiRead(const int fd)
{
  // a modifier, C'est juste pour voir tous les chaine lus
  static std::string fullStr;
  char buffer[12];
  int count = read(fd, buffer, sizeof(buffer));
  if (count == -1)
  {
    logger(LOG_ERROR, "read failed");
  } 
  else if (count == 0)
  {
    this->_pipeFdReadComplete = true;
    std::cout << fullStr << std::endl;
    logger(LOG_INFO, "It's finished");
  }
  else if (count > 0)
  {
    buffer[count] = 0;
    fullStr.append(buffer);
  }
}
