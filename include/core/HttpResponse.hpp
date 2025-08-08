/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:07:55 by srandria          #+#    #+#             */
/*   Updated: 2025/08/08 09:44:11 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "../utils/utils.hpp"

class HttpResponse
{
  private:
    HttpResponse(const HttpResponse& other);
    HttpResponse& operator=(const HttpResponse& other);

    int         _status_code;
    std::string _body;
    std::string _headers;
    size_t      _headersSize;
    size_t      _bodySize;
    size_t      _totalSent;

  public:
    HttpResponse(void);
    ~HttpResponse(void);

    void  setStatus(int code);
    void  setHeader(const std::string &content);
    void  setBody(const std::string &content);
    void  sendFile(const std::string &path);
    int   getStatus(void) const;
    void  saveHeadersBodySize(void);
    std::string build(void) const;  // Retourne la réponse HTTP complète
};

#endif
