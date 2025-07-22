/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 10:25:59 by srandria          #+#    #+#             */
/*   Updated: 2025/07/22 12:06:28 by srandria         ###   ########.fr       */
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
    Server(const Config& config);
    ~Server(void);

    void  stop_server(void);

  private:
    Server(void);
    Server(const Server &other);
    Server& operator=(const Server &other);

    int   createTcpSocket_(void);
    void  start_server_(void);
    void  create_all_listeners_(void);
    void  accept_new_client_(int listener_fd);
    void  handle_pollin_(int fd);
    void  handle_pollout_(int fd);
    void  close_client_(int fd);
    void  check_timout_(void);
    void  addFdToPoll_(int fd);
    void  startListener_(int fd, const ServerConfig &cfg);
    void  bindSocket_(int fd, const ServerConfig &cfg, struct sockaddr_in& addr);
    void  setSocketReuseAddr_(int fd);
    void  buildIpv4Sockaddr_(struct sockaddr_in& addr, const ServerConfig& cfg);
    void  setNonBlocking_(int fd);
    void  setPollOut_(int fd);



    const Config&               _config;
    std::map<int, Client*>      _clients;
    std::vector<struct pollfd>  _pool_fds;
    std::vector<int>            _listener_fds;

};

#endif
