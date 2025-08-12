/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 09:45:19 by srandria          #+#    #+#             */
/*   Updated: 2025/08/12 09:59:12 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP
 
#define COLOR_RESET   "\033[0m"
#define COLOR_INFO    "\033[36m"    // Cyan
#define COLOR_DEBUG   "\033[90m"    // Gris clair
#define COLOR_WARN    "\033[33m"    // Jaune
#define COLOR_ERROR   "\033[31m"    // Rouge
#define COLOR_FATAL   "\033[97;41m" // Blanc sur fond rouge

#include "../utils/Logger.hpp"
#include "../utils/contentType.hpp"

#include <iostream>
#include <string.h>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <utility>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include "cstdio"
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <poll.h>
#include <netdb.h>
#include <cerrno>
#include <fcntl.h>

template<typename T>
std::string toString(T value) { std::ostringstream oss;
    oss << value;
    return oss.str();
}

void        throwWithLog(LogLevel level, const std::string &msg);
std::string intToString(int value);
bool        caseInsensitiveEqual(const std::string& a,
            const std::string& b);
off_t       getFileSize(const std::string& path);

#endif
