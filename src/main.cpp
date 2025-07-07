/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 09:32:26 by srandria          #+#    #+#             */
/*   Updated: 2025/07/07 10:26:08 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/utils/utils.hpp"

int main(int argc, char **argv)
{
  (void)argv;
  if (argc != 2)
  {
    printError("2 argument are expected");
    return (0);
  }

  return (0);
}
