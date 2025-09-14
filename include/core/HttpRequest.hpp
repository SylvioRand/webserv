/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 12:54:14 by srandria          #+#    #+#             */
/*   Updated: 2025/09/14 14:19:45 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "../utils/utils.hpp"
#include "../../include/core/Config.hpp"

struct MultipartPart {
    bool          fullySaved;
    std::string   name;
    std::string   filename;
    std::string   contentType;
    std::string   data;
    size_t        offset;
};

class HttpRequest
{
  typedef typename std::vector<ServerConfig>::const_iterator  ServerConfigConstIterator;
  private:
    HttpRequest(const HttpRequest& other);
    HttpRequest& operator=(const HttpRequest &other);

    std::string                         _method;
    std::string                         _path;
    std::map<std::string, std::string>  _headers;
    std::string                         _body;
    std::string                         _version;
    bool                                _isComplete;
    size_t                              _bodyBytesRead;
    size_t                              _contentLength;
    ServerConfigConstIterator           _serverConf;
    bool                                _isChunked;
    std::string                         _bodyBuffChunked;
    bool                                _hasError;
    size_t                              _cgiOffset;
    bool                                _hasContentLength;
    bool                                _hasBoundary;
    std::string                         _boundary;
    std::string                         _endBoundary;
    std::vector<MultipartPart>          _multiPart;
    size_t                              _client_max_body_size;
    bool                                _isBodySizeAllowed;
    LocationConfig                      _location;

    void  parseHeader_(const std::string &raw_request, const size_t sizeOfHeader);
    void  handleMultipartFormData(const std::string& bodyPart);
    void  handleMultipartFormData(const char *bodyPart, const size_t len);
    void  handleChunkedEncoding(const std::string& bodyPart);
    void  handleChunkedEncoding(const char *bodyPart, const size_t len);
    void  handleFixedLengthBody(std::string& bodyPart);
    void  handleFixedLengthBody(const char *bodyPart, const size_t len);

  public:
    HttpRequest(void);
    ~HttpRequest(void);

    bool          _isCgiRequest;
    bool          _allFilesSaved;
    bool          _isReadingRequest;
    bool          _isBadRequest;

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
    void  setIsChunckedValue(void);
    bool  isChunked(void);
    void  extractBodyFromResponse(const std::string& bodyPart);
    bool  isNextChunkReady(size_t& contentSize);
    void  setError(void);
    bool  hasError(void);
    size_t
          getCgiOffset(void);
    void  sendRequestBodyToCgi(const int&pipeFd, const int& clientFd);
    void  addToMultipartStruct(size_t& start, size_t& end);
    bool  hasBoundary_(void);
    std::vector<MultipartPart>&
          getMultipart(void);
    void  setClientMaxBodySize(const size_t& size);
    LocationConfig
          getLocation(void);
    void  extractRequestBody(std::string& bodyPart);
    void  extractRequestBody(const char *data, size_t len);
    void  setHasContentLength(void);
    void  setHasBoundary(void);
    bool  hasContentLength(void);
    void  fillHeadersMap(std::istringstream& iss);
    void  parseMultipartBody();
};

#endif
