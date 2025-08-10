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
    ssize_t     _bodySize;
    ssize_t     _headersOffset;
    ssize_t     _bodyOffset;


  public:
    HttpResponse(void);
    ~HttpResponse(void);

    void  setStatus(int code);
    void  setHeader(const std::string &content);
    void  setBody(const std::string &content);
    void  sendFile(const std::string &path);
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
    int&   getBodyFileFd(void);
};

#endif
