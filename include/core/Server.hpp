/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 10:25:59 by srandria          #+#    #+#             */
/*   Updated: 2025/08/01 12:59:08 by srandria         ###   ########.fr       */
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
    typedef typename std::vector<ServerConfig>::const_iterator  ServerConfigConstIterator;
    typedef typename std::map<std::string, LocationConfig>::const_iterator  LocationConfigConstIterator;

    Server(const Config& config);
    ~Server(void);

    void  stop_server(void);
    std::string buildLocalPath(const std::string& uri, int fd);
    const Config& getConfig(void);

  private:
    Server(void);
    Server(const Server &other);
    Server& operator=(const Server &other);

    int   createTcpSocket_(void); void  start_server_(void); void  create_all_listeners_(void);
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
    bool  fileOk(const std::string localPath) const;
    const std::vector<ServerConfig>&  getServers(void);
    LocationConfig& getLocationConfig(const std::string uri);
    void  setCurrentLocation(LocationConfig& location);
    const LocationConfig& getCurrentLocation(void);
    bool  isMethodValid_(std::string method);
    void  setStatus(int code, int fd);
    void  error404(int fd);
    void  error403(int fd);
    void  error405(int fd);
    void  error400(void);
    void  error414(void);
    void  error413(void);
    void  error201(void);
    void  error500(void);
    void  error408(void);
    void  error505(void);
    std::string   getMethod(int fd);


    void  saveMatchingLocation_(const std::string& uri, ServerConfigConstIterator& cfg);
    ServerConfigConstIterator findMatchingServer(int fd);
    const std::map<std::string, std::string>& getHeaders(int fd);
    std::string getUri_(int fd);
    void  GETMethod(std::string& uri);


    


    const Config&                 _config;
    LocationConfig                _currentLocation;
    std::map<int, Client*>        _clients;
    std::vector<struct pollfd>    _pool_fds;
    std::vector<int>              _listener_fds;

};

#endif
