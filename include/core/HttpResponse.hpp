/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:07:55 by srandria          #+#    #+#             */
/*   Updated: 2025/09/13 16:17:35 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <ctime>
#include <fstream>
#include <iosfwd>
#define READ_CHUNK_SIZE 64000
//#define READ_CHUNK_SIZE 1600000

#include "../utils/utils.hpp"
#include <string>
#include <sys/types.h>

class HttpResponse
{
  private:
    HttpResponse(const HttpResponse& other);
    HttpResponse& operator=(const HttpResponse& other);

    int             _status_code;
    std::string     _bodyFilePath;
    std::ifstream   _bodyFileStream;
    std::string     _body;
    std::string     _headers;
    ssize_t         _headersSize;
    ssize_t         _headersOffset;
    ssize_t         _bodySize;
    char            _bodyBuffer[READ_CHUNK_SIZE];
    ssize_t         _bufferSize;
    ssize_t         _bufferOffset;
    ssize_t         _bodyBytesSent;
    bool            _keepAlive;
    std::string     _cgiResponse;
    ssize_t         _cgiResponseSize;
    ssize_t         _cgiBytesSent;;
    std::streampos  _streamOffset;

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
    //void  setBodyFileFd(const int& fd);
    std::string&
          getBodyFilePath(void);
    //int&  getBodyFileFd(void);
    void  setBodySize(const ssize_t& bodySize);
    void  initializeState(void);
    void  setKeepAliveStatus(bool value);
    bool  isKeepAlive(void);
    void  closeBodyFileStream(const int&fd);
    void  openAndSaveBodyFileStream(const std::string& path);

    // for cgi
    ssize_t
          getCgiBytesSent(void);
    ssize_t
          getCgiRespondSize(void);
    void  saveCgiRespondSize(const int& cliendFd);
    void  appendCgiResponse(const std::string& buff, const ssize_t& size);
    void  sendCgiResponse(const int&fd);
    bool  isCgiResponseFullySent(void);

    void  addExtraHeader(const std::string& connectionHeader,
        const std::string& version);

    bool        _isSending;
    bool        _isFullySent;
};

#endif
