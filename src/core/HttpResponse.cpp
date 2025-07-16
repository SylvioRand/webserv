/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:39:05 by srandria          #+#    #+#             */
/*   Updated: 2025/07/16 13:44:27 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/HttpResponse.hpp"

HttpResponse::HttpResponse(void) : _status_code(200)
{
}


HttpResponse::~HttpResponse(void)
{
}

void  HttpResponse::setStatus(int code)
{
  _status_code = code;
}

void  HttpResponse::setBody(const std::string &content)
{

}

void  HttpResponse::sendFile(const std::string &path)
{

}
