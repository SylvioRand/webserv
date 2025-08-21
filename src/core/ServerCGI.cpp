#include "../../include/core/Server.hpp"

void Server::handleCgiGetRequest_(std::string& path, const int fd)
{
  logger(LOG_DEBUG, "in handleCgiGetRequest_");
  while (1)
    ;
  (void)path;
  (void)fd;
}

void Server::handleCgiPostRequest_(std::string& path, const int fd)
{
  logger(LOG_DEBUG, "in handleCgiPostRequest_");
  (void)path;
  (void)fd;
}
