/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 12:54:14 by srandria          #+#    #+#             */
/*   Updated: 2025/08/25 13:11:21 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "../utils/utils.hpp"
#include "../../include/core/Config.hpp"
#include <sys/types.h>

class HttpRequest
{
  typedef typename std::vector<ServerConfig>::const_iterator  ServerConfigConstIterator;
  private:
    HttpRequest(const HttpRequest& other);
    HttpRequest& operator=(const HttpRequest &other);

    std::string                         _method;        // GET / POST / DELETE
    std::string                         _path;          // chemin se trouvant juste a cote de la methode dans la requete
    std::map<std::string, std::string>  _headers;       // entete de la requete http
    std::string                         _body;          // corps de la requet http
    std::string                         _version;       // HTTP/1.0, HTTP/1.1 etc...
    bool                                _isComplete;    // indique si la requete est completement recu
    size_t                              _bodyBytesRead; // nombre de bytes lu dans le body de la requete http
    size_t                              _contentLength;
    ServerConfigConstIterator           _serverConf;
    bool                                _isChunked;
    std::string                         _bodyBuffChunked;
    bool                                _hasError;
    size_t                              _cgiOffset;

    void  parseHeader_(const std::string &raw_request, const size_t sizeOfHeader);

  public:
    HttpRequest(void);
    ~HttpRequest(void);

    bool  _isCgiRequest;

    void  parse(const std::string &raw_request);
    bool  isValid(void) const;
    bool  isComplete(void) const;
    void  appendToBody(std::string& str);
    const std::string &getMethod(void) const;
    const std::string &getVersion(void) const;
    const std::string& getBody(void) const;
    const std::string& getPath(void);
    const std::map<std::string, std::string>& getHeaders(void) const;
    void  shiftBufferAfterRequest(void);
    bool  isBodySizeAllowed(void);
    void  markRequestComplete(void);
    void  setServerConf(ServerConfigConstIterator serverConf);
    ServerConfigConstIterator
      getServerConf(void);
    LocationConfig
      getMatchingLocation_(const std::string& uri, const ServerConfigConstIterator& cfg);
    LocationConfig
      createAndReturnRootLocation_(const ServerConfigConstIterator& cfg);
    void  setIsChunckedValue(void);
    bool  isChunked(void);
    void  parseBody(void);
    void  extractBodyFromResponse(const std::string& bodyPart);
    bool  isNextChunkReady(size_t& contentSize);
    void  setError(void);
    bool  hasError(void);
    size_t   getCgiOffset(void);
    void  sendRequestBodyToCgi(const int&pipeFd, const int& clientFd);
};

#endif
