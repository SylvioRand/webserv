/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 10:25:59 by srandria          #+#    #+#             */
/*   Updated: 2025/09/11 11:02:52 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP
#include "../utils/utils.hpp"
#include "../core/Config.hpp"
#include "../core/Client.hpp"
#include <string>


struct CgiPipes
{
  int in_pipe[2];
  int out_pipe[2];
};

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

    int   createTcpSocket_(void); void  start_server_(void); void  create_all_listeners_(void);
    void  accept_new_client_(int listener_fd);
    void  handle_pollin_(int fd, bool& isClientClosed);
    void  handle_pollout_(int fd, bool& isClientClosed);
    void  close_client_(int fd);
    void  close_pipeFd_(const int& fd);
    void  check_timout_(void);
    void  addFdToPoll_(int fd);
    void  startListener_(int fd, ServerConfigConstIterator );
    void  bindSocket_(int fd, const ServerConfig &cfg, struct sockaddr_in& addr);
    void  setSocketReuseAddr_(int fd);
    void  buildIpv4Sockaddr_(struct sockaddr_in& addr, const ServerConfig& cfg);
    void  setNonBlocking_(int fd);
    void  setPollOut_(int fd);
    const std::vector<ServerConfig>&
          getServers(void);
    LocationConfig&
          getLocationConfig(const std::string uri);
    void  setCurrentLocation(LocationConfig& location);
    const LocationConfig&
          getCurrentLocation(void);
    bool  isHttpMethodValid_(std::string method);
    bool  isMethodAllowedForLocation(const std::string method);
    bool  isSupportedHttpMethod(const std::string& method);
    void  setStatus(int code, int fd);
    int   getStatus(int code);
    std::string
          getMethod(int fd);
    std::string
          getVersion(int fd);
    void  saveMatchingLocation_(const int& fd, ServerConfigConstIterator& cfg);
    const std::map<std::string, std::string>& getHeaders(int fd);
    const std::string&
          getRequestUri_(const int& fd);
    void  GETMethod_(const int& fd);
    void  POSTMethod_(const int fd);
    void  DELETEMethod_(const int fd);
    bool  isFile_(const std::string localPath) const;
    bool  isReadable_(const std::string localPath) const;
    void  buildDirectoryListing_(const int fd);
    void  processReadableFile_(const int fd, const std::string& path);
    bool  directoryExists_(const std::string& path);
    void  onDeleteSuccess_(const int fd);
    void  cannotDeleteFile_(const int fd, std::string& path);
    void  respondFileNotReadable(const int fd);
    void  respondDirectoryListingForbidden(const int fd);
    void  respondNotFound_(const int fd);
    void  badRequest_(const int fd);
    void  methodNotAllowed_(const int fd);
    bool  hasIndexDirective_(void);
    std::string
          getAccessibleIndexPath_(const std::string& path);
    void  serveIndexContent_(const std::string path, const int fd);
    bool  existsAtLeastOneIndexFile_(const std::string path);
    void  respondIndexFilesUnreadable_(const int fd);
    void  respondNoIndexFileFound_(const int fd);
    void  respondMissingUploadDir(const int fd);
    void  respondWithUploadError(const int fd);
    void  saveUploadedFile_(const int fd);
    std::string
          getAllowedMethodsForLocation(void);
    std::string
          buildConnectionHeader(const int fd);
    void  handleNoMatchingLocation_(const int fd);
    bool  hasCustomErrorPage(const int code, const int fd);
    void  saveErrorBodyFilePath(const int code, const int& fd,
        std::string& contentType, std::string& contentLength);
    std::string
          readLocalFileToString(std::string path);
    void loadMimeTypes(void);
    std::string
          getContentTypeByFileExtension(std::string path);
    void  saveHeaderAndBodySize(const int& fd);
    void  handleRedirect_(const int& fd);
    bool  isRedirectCode_(int statusCode);
    void  respondRedirect_(const int& fd,
        const std::map<int, std::string>::const_iterator it);
    void  handleReturnWithoutUrl_(const int& fd);
    bool  isValidHttpStatusCode_(const int& code);
    void  respondNotImplemented_(const int& fd);
    void  setBodyFilePath(const int& fd, const std::string& path);
    //void  createAndSaveRootLocation_(ServerConfigConstIterator& cfg);
    void  setBodySize(const int& fd, const ssize_t& bodySize);
    void  setPollIn_(const int& fd);
    void  respondPayloadTooLarge(const int& fd);
    void  saveBodyToBinary(const int& fd);
    std::string
          getUriPath_(const int& fd);
    void  respondFallbackError(const int& fd);
    void  saveMultipartFiles(const int&fd);
    bool  isBodySizeAllowed(const int& fd);

    // for cgi
    void  handleCgiGetRequest_(const int fd);
    void  setIsCGIRequest(const int& fd);
    void  respondInternalServerError(const int&fd);
    void  prepareAndLaunchCGI(const int& fd);
    void  launchCgiProcess(const int& fd, const std::string& localPath);
    std::string
          getFileExtension_(std::string path);
    bool  isExecutable_(const std::string& path);
    void  respondNotExecutable(const int& fd);
    char  **buildEnvpForExecve_(const int& fd);
    void  readCgiResponse(const int& pipeFd, const int& clientFd);
    const std::string getFileName(const std::string uriPath);
    void  unregisterCgiFd(const int& fd);
    const std::string generateAutoIndexHtml(const int&fd);
    void  handleChildProcess(const int& fd, const std::string& localPath, const CgiPipes& cgiPipes);
    void  handleParentProcess(const int& fd, const CgiPipes& cgiPipes);
    void  sendRequestBodyToCgi(const int&fd, const int& clientFd);
    bool  checkShutdownRequest(void);

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
