/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:24:24 by srandria          #+#    #+#             */
/*   Updated: 2025/07/16 15:23:23 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/Client.hpp"

Client::Client(int fd) : _fd(fd), _lastActivity(time(NULL))
{
  fcntl(_fd, F_SETFL, O_NONBLOCK); // Mode non-bloquant
}

Client::~Client(void)
{
  if (_fd != -1) {
    close(_fd);
  }
}

// TODO
void  Client::readData(void)
{

}

// TODO
void  Client::sendData(void)
{

}

bool  Client::isRequestComplete(void) const
{
  return (_request.isComplete());
}
