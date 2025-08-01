/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:39:05 by srandria          #+#    #+#             */
/*   Updated: 2025/08/01 09:59:57 by srandria         ###   ########.fr       */
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

// TODO
void  HttpResponse::setBody(const std::string &content)
{
  (void)content;
}

// TODO
void  HttpResponse::sendFile(const std::string &path)
{
  (void)path;
}

// TODO
std::string HttpResponse::build(void) const
{
  std::string result;
  return (result);
}

int   HttpResponse::getStatus(void) const
{
  return (_status_code);
}
