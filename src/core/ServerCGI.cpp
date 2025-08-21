#include "../../include/core/Server.hpp"

void Server::handleCgiGetRequest_(const int fd)
{
  logger(LOG_INFO, "getRequestUri_-> " + this->getRequestUri_(fd));
  logger(LOG_INFO, "getUriPath_->" + this->getUriPath_(fd));
  logger(LOG_DEBUG, "in handleCgiGetRequest_");
  (void)fd;
}

void Server::handleCgiPostRequest_(const int fd)
{
  logger(LOG_DEBUG, "in handleCgiPostRequest_");
  (void)fd;
}
