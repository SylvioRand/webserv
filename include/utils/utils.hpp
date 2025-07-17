/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 09:45:19 by srandria          #+#    #+#             */
/*   Updated: 2025/07/17 08:49:40 by srandria         ###   ########.fr       */
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
#include <ostream>
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
