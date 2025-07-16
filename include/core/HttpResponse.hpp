/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:07:55 by srandria          #+#    #+#             */
/*   Updated: 2025/07/16 13:09:51 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "../utils/utils.hpp"

class HttpResponse
{
  private:
    int         _status_code;
    std::string _body;
    std::string _headers;

  public:
    void setStatus(int code);
    void setBody(const std::string &content);
    void sendFile(const std::string &path);
    std::string build() const;  // Retourne la réponse HTTP complète
};

#endif
