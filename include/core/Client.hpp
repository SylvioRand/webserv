/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 12:18:58 by srandria          #+#    #+#             */
/*   Updated: 2025/09/10 12:28:52 by zramahaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../core/HttpRequest.hpp"
#include "../core/HttpResponse.hpp"
#include "Config.hpp"
#include <string>

class Client
{
  public:
    typedef std::vector<ServerConfig>::const_iterator  ServerConfigConstIterator;

    Client(int fd, ServerConfigConstIterator cfg_ite);
    ~Client(void);

    bool  _isReadingCgiResponse;

    bool  readData(void);
    void  sendData(void);
    void  sendCgiData(void);
    bool  isRequestComplete(void) const;
    HttpRequest& getRequest(void);
    HttpResponse& getResponse(void);
    ServerConfigConstIterator getServerConfig(void) const;
    void  clearBuffer(void);
    void  setPath(const std::string);
    const std::string& getPath(void);
    void  setCurrentLocation(LocationConfig& location);

  private:
    Client(void);
    Client(const Client &other);
    Client& operator=(const Client& other);


    int                       _fd;
    HttpRequest               _request;
    HttpResponse              _response;
    std::string               _buffer;
    time_t                    _lastActivity; // Pour timeout
    ServerConfigConstIterator _cfg;
    std::string               _path;
    LocationConfig            _currentLocation;
    

};

#endif
