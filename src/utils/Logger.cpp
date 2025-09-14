/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 08:23:20 by srandria          #+#    #+#             */
/*   Updated: 2025/09/14 14:32:02 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/utils/utils.hpp"

void logger(LogLevel level, const std::string &message)
{
  #if PRODUCTION_MODE
    if (level == LOG_DEBUG)
      return ;
  #endif

  const std::string prefix[5] = {"INFO ", "DEBUG", "WARNING", "ERROR", "FATAL"};
  std::ostream& out = (level > 1) ? std::cerr : std::cout;

  switch (level)
  {
    case 0: // INFO
      out << COLOR_INFO << "[" << prefix[level] << "] " << message << COLOR_RESET << std::endl;
      break;
    case 1: // DEBUG
    case 2: // WARNING
      out << COLOR_DEBUG << "[" << prefix[level] << "] " << message << COLOR_RESET << std::endl;
      break;
    case 3: // ERROR
      out << COLOR_ERROR << "[" << prefix[level] << "] " << message << COLOR_RESET << std::endl;
      break;
    case 4: // FATAL
      out << COLOR_FATAL << "[" << prefix[level] << "] " << message << COLOR_RESET << std::endl;
      break;
    default:
      out << "[" << prefix[level] << "] " << message << std::endl;
      break;
  }
}
