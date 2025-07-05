#ifndef SERVER_HPP
#define SERVER_HPP

#include "../utils/utils.hpp"

class Server
{
  public:
    Server(int port, const std::string &host);
    ~Server(void);

    void  initSocket();

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
