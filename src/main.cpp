/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 09:32:26 by srandria          #+#    #+#             */
/*   Updated: 2025/07/21 10:19:55 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/utils/utils.hpp"
#include "../include/core/Server.hpp"
#include <signal.h>

int main(int argc, char **argv)
{
    // signal(SIGPIPE, SIG_IGN); // don`t need this`
  (void)argv;
  if (argc != 2)
  {
    logger(LOG_ERROR, "2 argument are expected");
    return (0);
  }

  try {
    Config  config(argv[1]);
    Server  server(config);

  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return (0);
  }

  return (0);
}
