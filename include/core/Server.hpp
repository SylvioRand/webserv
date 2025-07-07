/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 10:25:59 by srandria          #+#    #+#             */
/*   Updated: 2025/07/07 13:17:59 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "../utils/utils.hpp"

class Server
{
  public:
    Server(int port, const std::string &host);
    ~Server(void);

    void  initSocket();
    void  startListening();
    int   getSocketFd(void);

  private:
    Server(void);
    Server(const Server &other);
    Server& operator=(const Server& other);

    int                 _socketFd;
    int                 _port;
    std::string         _host;
    struct sockaddr_in  _address;

};

#endif
