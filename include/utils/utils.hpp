/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 09:45:19 by srandria          #+#    #+#             */
/*   Updated: 2025/09/14 14:26:35 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP
 
#define COLOR_RESET   "\033[0m"
#define COLOR_INFO    "\033[1;34m"
#define COLOR_DEBUG   "\033[1;33m"
#define COLOR_WARN    "\033[1;38;5;208m"
#define COLOR_ERROR   "\033[1;31m"
#define COLOR_FATAL   "\033[1;97;41m"

#include "../utils/Logger.hpp"
#include "../utils/contentType.hpp"
#include "../utils/timeouts.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <cstring>
#include <sys/socket.h>
#include <poll.h>
#include <fcntl.h>
#include <ctime>
#include <dirent.h>
#include <csignal>
#include <sys/wait.h>

extern volatile sig_atomic_t g_shouldStop;

template<typename T>
std::string toString(T value)
{
  std::ostringstream oss;
  
  oss << value;
  return oss.str();
}

void        signalHandler(int signal);
void        throwWithLog(LogLevel level, const std::string &msg);
std::string intToString(int value);
bool        caseInsensitiveEqual(const std::string& a,
            const std::string& b);
off_t       getFileSize(const std::string& path);
std::string toLower(const std::string &s);
std::string toUpper(const std::string &s);
std::string unique_filename(const std::string &original);

#endif
