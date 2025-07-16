/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 12:54:14 by srandria          #+#    #+#             */
/*   Updated: 2025/07/16 15:22:24 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "../utils/utils.hpp"

class HttpRequest
{
  private:
    HttpRequest(const HttpRequest& other);
    HttpRequest& operator=(const HttpRequest &other);

    std::string                         _method;      // GET / POST / DELETE
    std::string                         _path;        // chemin se trouvant pres la methode dans la requete
    std::map<std::string, std::string>  _headers;     // entete de la requete http
    std::string                         _body;        // corps de la requet http
    std::string                         _version;     // HTTP/1.0, HTTP/1.1 etc...
    bool                                _isComplete;  // indique si la requete est completement recu

  public:
    HttpRequest(void);
    ~HttpRequest(void);

    void  parse(const std::string &raw_request);
    bool  isValid(void) const;
    const std::string &getMethod() const;
    bool  isComplete(void) const;
};

#endif
