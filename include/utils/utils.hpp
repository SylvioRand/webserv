/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 09:45:19 by srandria          #+#    #+#             */
/*   Updated: 2025/07/29 16:48:27 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include "../utils/Logger.hpp"

#include <iostream>
#include <string.h>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <utility>
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

void        throwWithLog(LogLevel level, const std::string &msg);
std::string intToString(int value);


#endif
