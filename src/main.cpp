/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 09:32:26 by srandria          #+#    #+#             */
/*   Updated: 2025/07/17 09:43:13 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/utils/utils.hpp"
#include "../include/core/Server.hpp"

int main(int argc, char **argv)
{
  (void)argv;
  if (argc != 2)
  {
    logger(LOG_ERROR, "2 argument are expected");
    return (0);
  }

  /*
  Multiplexer mux;
  mux.addListeningSockets(server.getListenFds());

  mux.loop();
  */

  return (0);
}
