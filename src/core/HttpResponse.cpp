/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:39:05 by srandria          #+#    #+#             */
/*   Updated: 2025/08/08 09:47:40 by srandria         ###   ########.fr       */
/*                                                                            */ /* ************************************************************************** */
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
  this->_body= content;
}

void  HttpResponse::setHeader(const std::string &content)
{
  this->_headers = content;
}


// TODO
void  HttpResponse::sendFile(const std::string &path)
{
  (void)path;
}

std::string HttpResponse::build(void) const
{
  return (this->_headers + "\r\n" + this->_body);
}

int   HttpResponse::getStatus(void) const
{
  return (_status_code);
}

void  HttpResponse::saveHeadersBodySize(void)
{
  if (!this->_bodySize)
    this->_bodySize = this->_body.size();
  this->_headersSize = this->_headers.size();
}

