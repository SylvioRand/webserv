/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 12:18:58 by srandria          #+#    #+#             */
/*   Updated: 2025/07/16 13:27:47 by srandria         ###   ########.fr       */
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
    int           _fd;
    HttpRequest   _request;
    HttpResponse  _response;
    std::string   _buffer;

    Client(void);
    Client(const Client &other);
    Client& operator=(const Client& other);

};

#endif
