/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:24:24 by srandria          #+#    #+#             */
/*   Updated: 2025/07/16 13:28:35 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/Client.hpp"

Client::Client(int fd) : _fd(fd)
{
}

Client::~Client(void)
{

}

void  Client::readData(void)
{

}

void  Client::sendData(void)
{

}

bool  Client::isRequestComplete(void) const
{
  return (true);
}
