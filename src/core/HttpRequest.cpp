/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:30:00 by srandria          #+#    #+#             */
/*   Updated: 2025/07/17 09:47:53 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/HttpRequest.hpp"

HttpRequest::HttpRequest(void)
{

}

HttpRequest::~HttpRequest(void)
{

}

// TODO At this part, we need to verify if all content of the body has been read, at the same time and set the variable _isComplete as true if it`s the case`
void  HttpRequest::parse(const std::string &raw_request)
{
  (void)raw_request;
  // we need to add number of bytes for body part on = the variable _bodyBytesRead

}

bool  HttpRequest::isValid(void) const
{
  return (true);
}

const std::string& HttpRequest::getMethod(void) const
{
  return (_method);
}

bool  HttpRequest::isComplete(void) const
{
  return (_isComplete);
}
