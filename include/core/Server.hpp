/* ************************************************************************* */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 10:25:59 by srandria          #+#    #+#             */
/*   Updated: 2025/09/15 10:35:12 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP
#include "../utils/utils.hpp"
#include "../core/Config.hpp"
#include "../core/Client.hpp"

struct CgiPipes
{
  int in_pipe[2];
  int out_pipe[2];
};

// timeout in sec
namespace timeout {
    const int CLIENT_HEADER_TIMEOUT = 5;
    const int CLIENT_BODY_TIMEOUT   = 10;
    const int SEND_TIMEOUT          = 10;
    const int KEEPALIVE_TIMEOUT     = 15;
    const int PROXY_CONNECT_TIMEOUT = 3;
    const int PROXY_READ_TIMEOUT    = 15;
    const int PROXY_SEND_TIMEOUT    = 15;
}

class Server
{
  public:
    typedef typename std::vector<ServerConfig>::const_iterator  ServerConfigConstIterator;
    typedef typename std::map<std::string, LocationConfig>::const_iterator  LocationConfigConstIterator;

    Server(const Config& config);
    ~Server(void);

    void  stop_server(void);
    const Config& getConfig(void);

  private: Server(void); Server(const Server &other); Server& operator=(const Server &other);

    int   createTcpSocket_(void);
    void  start_server_(void);
    void  create_all_listeners_(void);
    void  accept_new_client_(const int& listener_fd);
    void  handle_pollin_(const int& fd, bool& isClientClosed);
    void  handle_pollout_(const int& fd, bool& isClientClosed);
    void  close_client_(const int& fd);
    void  close_pipeFd_(const int& fd);
    void  addFdToPoll_(const int& fd);
    void  startListener_(const int& fd, ServerConfigConstIterator );
    void  bindSocket_(const int& fd, const ServerConfig &cfg, struct sockaddr_in& addr);
    void  setSocketReuseAddr_(const int& fd);
    void  buildIpv4Sockaddr_(struct sockaddr_in& addr, const ServerConfig& cfg);
    void  setNonBlocking_(const int& fd);
    void  setPollOut_(const int& fd);
    const std::vector<ServerConfig>&
          getServers_(void);
    LocationConfig&
          getLocationConfig_(const std::string& uri);
    void  setCurrentLocation_(LocationConfig& location);
    const LocationConfig&
          getCurrentLocation_(void);
    bool  isHttpMethodValid_(const std::string& method);
    bool  isMethodAllowedForLocation_(const std::string& method);
    bool  isSupportedHttpMethod_(const std::string& method);
    void  setStatus_(const int& code, const int& fd);
    int   getStatus_(const int& code);
    std::string
          getMethod_(const int& fd);
    std::string
          getVersion_(const int& fd);
    void  saveMatchingLocation_(const int& fd, ServerConfigConstIterator& cfg);
    const std::map<std::string, std::string>& getHeaders_(const int& fd);
    const std::string&
          getRequestUri_(const int& fd);
    void  GETMethod_(const int& fd);
    void  POSTMethod_(const int& fd);
    void  DELETEMethod_(const int& fd);
    bool  isFile_(const std::string& localPath) const;
    bool  isReadable_(const std::string& localPath) const;
    void  buildDirectoryListing_(const int& fd);
    void  processReadableFile_(const int& fd, const std::string& path);
    bool  directoryExists_(const std::string& path);
    void  onDeleteSuccess_(const int& fd);
    void  cannotDeleteFile_(const int& fd, const std::string& path);
    void  respondFileNotReadable_(const int& fd);
    void  respondDirectoryListingForbidden_(const int& fd);
    void  respondNotFound_(const int& fd);
    void  badRequest_(const int& fd);
    void  methodNotAllowed_(const int& fd);
    bool  hasIndexDirective_(void);
    std::string
          getAccessibleIndexPath_(const std::string& path);
    void  serveIndexContent_(const std::string& path, const int& fd);
    bool  existsAtLeastOneIndexFile_(const std::string& path);
    void  respondIndexFilesUnreadable_(const int& fd);
    void  respondNoIndexFileFound_(const int& fd);
    void  respondMissingUploadDir_(const int& fd);
    void  respondWithUploadError_(const int& fd);
    void  saveUploadedFile_(const int& fd);
    std::string
          getAllowedMethodsForLocation_(void);
    std::string
          buildConnectionHeader_(const int& fd);
    void  handleNoMatchingLocation_(const int& fd);
    bool  hasCustomErrorPage_(const int& code, const int& fd);
    void  saveErrorBodyFilePath_(const int& code, const int& fd,
          std::string& contentType, std::string& contentLength);
    std::string
          readLocalFileToString_(const std::string& path);
    void loadMimeTypes_(void);
    std::string
          getContentTypeByFileExtension_(std::string path);
    void  saveHeaderAndBodySize_(const int& fd);
    void  handleRedirect_(const int& fd);
    bool  isRedirectCode_(const int& statusCode);
    void  respondRedirect_(const int& fd,
          const std::map<int, std::string>::const_iterator it);
    void  handleReturnWithoutUrl_(const int& fd);
    bool  isValidHttpStatusCode_(const int& code);
    void  respondNotImplemented_(const int& fd);
    void  setBodyFilePath_(const int& fd, const std::string& path);
    void  setBodySize_(const int& fd, const ssize_t& bodySize);
    void  setPollIn_(const int& fd);
    void  respondPayloadTooLarge_(const int& fd);
    void  saveBodyToBinary_(const int& fd);
    std::string
          getUriPath_(const int& fd);
    void  respondFallbackError_(const int& fd);
    void  saveMultipartFiles_(const int& fd);
    bool  isBodySizeAllowed_(const int& fd);

    // for cgi
    void  handleCgiGetRequest_(const int& fd);
    void  setIsCGIRequest_(const int& fd);
    void  respondInternalServerError_(const int&fd);
    void  prepareAndLaunchCGI_(const int& fd);
    void  launchCgiProcess_(const int& fd, const std::string& localPath);
    std::string
          getFileExtension_(const std::string& path);
    bool  isExecutable_(const std::string& path);
    void  respondNotExecutable_(const int& fd);
    char  **buildEnvpForExecve_(const int& fd);
    void  readCgiResponse_(const int& pipeFd, const int& clientFd);
    const std::string getFileName_(const std::string& uriPath);
    void  unregisterCgiFd_(const int& fd);
    const std::string generateAutoIndexHtml_(const int& fd);
    void  handleChildProcess_(const int& fd, const std::string& localPath, const CgiPipes& cgiPipes);
    void  handleParentProcess_(const int& fd, const CgiPipes& cgiPipes);
    void  sendRequestBodyToCgi_(const int&fd, const int& clientFd);
    bool  checkShutdownRequest_(void);
    void  respondForbidden_(const int& fd);
    void  handleServerTimeout_(void);
    void  handleClientHeaderTimeout_(const int& fd);
    void  respond408RequestTimeout_(const int& fd);
    void  handleClientBodyTimeout_(const int& fd);
    void  handleKeepAliveTimeout_(const int& fd);
    void  handleSendTimeout_(const int& fd);
    void  handleFinishedChildren_(void);
    void  respondBinaryNotFound_(const int&fd);

    const Config&                             _config;
    LocationConfig                            _currentLocation;
    std::map<int, Client*>                    _clients;
    std::vector<struct pollfd>                _pool_fds;
    std::vector<int>                          _listener_fds;
    std::map<std::string, std::string>        _mimes;
    std::string                               _localPath;
    std::map<int, ServerConfigConstIterator>  _serverListeners;
    std::vector<int>                          _pipeFd;
    std::map<int, int>                        _pipeFdClient;
};

#endif
