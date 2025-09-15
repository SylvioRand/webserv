/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 06:52:18 by srandria          #+#    #+#             */
/*   Updated: 2025/09/09 08:54:15 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/utils/utils.hpp"
#include "../../include/core/Server.hpp"

volatile sig_atomic_t g_shouldStop = 0;

void signalHandler(int signal)
{
  std::cout << "\b\b  \b\b";
  if (signal == SIGINT)
    logger(LOG_INFO, "SIGINT received (Ctrl+C).");
  else
    logger(LOG_INFO, "SIGQUIT received (Ctrl+C).");
  g_shouldStop = 1;
}

