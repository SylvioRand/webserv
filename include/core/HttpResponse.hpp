/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:07:55 by srandria          #+#    #+#             */
/*   Updated: 2025/08/08 14:26:47 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#define READ_CHUNK_SIZE 8192

#include "../utils/utils.hpp"
#include <string>
#include <sys/types.h>

class HttpResponse
{
  private:
    HttpResponse(const HttpResponse& other);
    HttpResponse& operator=(const HttpResponse& other);

    int         _status_code;
    std::string _bodyFilePath;
    int         _bodyFileFd;
    std::string _body;
    std::string _headers;
    ssize_t     _headersSize;
    ssize_t     _headersOffset;
    ssize_t     _bodySize;
    char        _bodyBuffer[READ_CHUNK_SIZE];
    ssize_t     _bufferSize;
    ssize_t     _bufferOffset;
    ssize_t     _bodyBytesSent;
    bool        _keepAlive;
    std::string _cgiResponse;
    ssize_t     _cgiResponseSize;
    ssize_t     _cgiBytesSent;;


  public:
    HttpResponse(void);
    ~HttpResponse(void);

    void  setStatus(int code);
    void  setHeader(const std::string &content);
    void  setBody(const std::string &content);
    int   getStatus(void) const;
    void  saveHeadersAndBodySize(void);
    std::string
          build(void) const;  // Retourne la réponse HTTP complète
    void  sendHeaders(const int& fd);
    void  sendBody(const int& fd);
    bool  areHeadersFullySent();
    bool  isBodyFullySent();
    void  setBodyFilePath(const std::string path);
    void  setBodyFileFd(const int& fd);
    std::string&
          getBodyFilePath(void);
    int&  getBodyFileFd(void);
    void  setBodySize(const ssize_t& bodySize);
    void  initializeState(void);
    void  setKeepAliveStatus(bool value);
    bool  isKeepAlive(void);
    ssize_t
          getCgiBytesSent(void);
    ssize_t
          getCgiRespondSize(void);
    void  saveCgiRespondSize(const int& cliendFd);
    void  appendCgiResponse(const std::string& buff, const ssize_t& size);
    void  sendCgiResponse(const int&fd);
    bool  isCgiResponseFullySent(void);
    void  closeBodyFileFd(const int&fd, const std::string path);

    bool  _isFullySent;
};

#endif
