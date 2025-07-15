#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <string.h>
#include <stdexcept>
#include <unistd.h>
#include <cstring>
#include "cstdio"
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <poll.h>
#include <netdb.h>
#include <cerrno>
#include <fcntl.h>


void  printError(std::string message);

#endif
