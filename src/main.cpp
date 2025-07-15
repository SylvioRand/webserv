/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 09:32:26 by srandria          #+#    #+#             */
/*   Updated: 2025/07/15 12:37:43 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/utils/utils.hpp"
#include "../include/core/Server.hpp"

int main(int argc, char **argv)
{
  (void)argv;
  if (argc != 2)
  {
    printError("2 argument are expected");
    return (0);
  }
  Server server;

    // Exemple : on écoute sur 2 adresses/ports
  server.addListen("127.0.0.1", 8080);
  server.addListen("::1", 8080); // IPv6 loopback

  try
  {
      server.initSockets();
      server.startListening();
  }
  catch (const std::exception &e)
  {
      std::cerr << "Server setup error: " << e.what() << std::endl;
      return 1;
  }

  /*
  Multiplexer mux;
  mux.addListeningSockets(server.getListenFds());

  mux.loop();
  */

  return (0);
}
