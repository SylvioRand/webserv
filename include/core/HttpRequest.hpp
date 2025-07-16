/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 12:54:14 by srandria          #+#    #+#             */
/*   Updated: 2025/07/16 13:02:01 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "../utils/utils.hpp"

class HttpRequest
{
  private:
    std::string                         _method;
    std::string                         _path;
    std::map<std::string, std::string>  _headers;

  public:
    void  parse(const std::string &raw_request);
    bool  isValid() const;
    const std::string &getMethod() const;
};

#endif
