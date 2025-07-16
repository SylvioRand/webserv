/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 10:25:59 by srandria          #+#    #+#             */
/*   Updated: 2025/07/16 12:58:40 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "../utils/utils.hpp"
#include "../core/Config.hpp"
#include "../core/Client.hpp"

class Server
{
  public:
    Server(const struct ServerConfig);


  private:
    Server(void);
    ~Server(void);
    Server(const Server &other);
    Server& operator=(const Server &other);

    Config  _config;
    std::map<int, Client> _clients;
    std::vector<struct pollfd> _fds;

};

/*
class Server
{
  public:
    Server(void);
    ~Server(void);

    void addListen(const std::string &host, int port);
    void initSockets();
    void startListening();

    const std::vector<int>& getListenFds() const;

  private:
    Server(const Server &other);
    Server& operator=(const Server &other);

    struct ListenInfo {
        int                 socketFd;
        int                 port;
        std::string         host;
        int                 family; // AF_INET or AF_INET6
        struct sockaddr_storage address;
        socklen_t           addrLen;
    };

    std::vector<ListenInfo> _listen_fds;
};
*/

#endif

