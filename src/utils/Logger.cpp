/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 08:23:20 by srandria          #+#    #+#             */
/*   Updated: 2025/07/17 09:13:53 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/utils/utils.hpp"

void logger(LogLevel level, const std::string &message)
{
  const std::string prefix[5] = {"INFO", "DEBUG", "WARNING", "ERROR", "FATAL"};
  std::ostream& out = (level > 1) ? std::cerr : std::cout;

  if (level == 0)
    out << COLOR_INFO << "[" << prefix[level] << "] " << message << COLOR_RESET << std::endl;
  else if (level == 1)
    out <<  "[" << prefix[level] << "] " << message << std::endl;
  else if (level == 2)
    out << COLOR_DEBUG << "[" << prefix[level] << "] " << message << COLOR_RESET << std::endl;
  else if (level == 3)
    out << COLOR_ERROR << "[" << prefix[level] << "] " << message << COLOR_RESET << std::endl;
  else if (level == 4)
    out << COLOR_FATAL << "[" << prefix[level] << "] " << message << COLOR_RESET <<  std::endl;
}
