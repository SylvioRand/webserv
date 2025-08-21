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
  std::string absolutePath = this->getCurrentLocation().root  + "/" + getUriPath_(fd).substr(this->getCurrentLocation().path.size());
  logger(LOG_DEBUG, "in handleCgiGetRequest_");
  logger(LOG_INFO, "getRequestUri_-> " + this->getRequestUri_(fd));
  logger(LOG_INFO, "getUriPath_-> " + this->getUriPath_(fd));
  logger(LOG_INFO, "absolutePath-> " + absolutePath);

  // error cgi_path
  if (!this->isFile_(this->getCurrentLocation().cgi_path))
  {
    logger(LOG_ERROR, "500 Internal Server Error: CGI interpreter not available");
  }
  // build env variables
  char **envp = this->buildEnvpForExecve_(fd);
  (void)envp;

  // execute the script and cummunicate with him
  int cgi_pipe[2];
  if (pipe(cgi_pipe) == -1)
  {
    logger(LOG_FATAL, "Error FATAL");
  }
  std::cout << "cgi_pipe[0] = " << cgi_pipe[0] << std::endl;
  std::cout << "cgi_pipe[1] = " << cgi_pipe[1] << std::endl;
  // TODO: a la maison
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
