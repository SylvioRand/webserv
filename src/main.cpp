/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 09:32:26 by srandria          #+#    #+#             */
/*   Updated: 2025/07/07 13:13:48 by srandria         ###   ########.fr       */
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
  /*
  Server server(4242, "localhost");
  server.initSocket();
  server.startListening();
  */
  return (0);
}
