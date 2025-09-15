/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   throwWithLog.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 08:09:35 by srandria          #+#    #+#             */
/*   Updated: 2025/07/21 13:40:58 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/utils/utils.hpp"

// TODO : Call a cleanup function to close FDs and free resources (prevent leaks)
void  throwWithLog(LogLevel level, const std::string &msg)
{
  const std::string prefix[5] = {"INFO ", "DEBUG", "WARNING", "ERROR", "FATAL"};

  if (level < LOG_INFO || level > LOG_FATAL)
    throw std::runtime_error("UNKNOWN log level: " + msg);

  std::string msglevel = "[" + prefix[level] + "] " + msg;

  throw std::runtime_error(msglevel);
}
