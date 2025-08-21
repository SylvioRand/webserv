#include "../../include/core/Server.hpp"

void Server::handleCgiGetRequest_(std::string& path, const int fd)
{
  logger(LOG_INFO, path);
  logger(LOG_INFO, this->getPath_(fd));
  logger(LOG_INFO, this->getUri_(fd));
  logger(LOG_DEBUG, "in handleCgiGetRequest_");
  (void)path;
  (void)fd;
}

void Server::handleCgiPostRequest_(std::string& path, const int fd)
{
  logger(LOG_DEBUG, "in handleCgiPostRequest_");
  (void)path;
  (void)fd;
}
