/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 12:18:58 by srandria          #+#    #+#             */
/*   Updated: 2025/07/22 12:29:30 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../core/HttpRequest.hpp"
#include "../core/HttpResponse.hpp"

class Client
{
  public:
    Client(int fd);
    ~Client(void);

    void readData(void);
    void sendData(void);
    bool isRequestComplete(void) const;

  private:
    Client(void);
    Client(const Client &other);
    Client& operator=(const Client& other);

    int           _fd;
    HttpRequest   _request;
    HttpResponse  _response;
    std::string   _buffer;
    time_t        _lastActivity; // Pour timeout

};

#endif
