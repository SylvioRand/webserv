/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 13:30:00 by srandria          #+#    #+#             */
/*   Updated: 2025/07/16 15:22:39 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/HttpRequest.hpp"

HttpRequest::HttpRequest(void)
{

}

HttpRequest::~HttpRequest(void)
{

}

// TODO
void  HttpRequest::parse(const std::string &raw_request)
{

}

// TODO
bool  HttpRequest::isValid(void) const
{
  return (true);
}

const std::string& HttpRequest::getMethod() const
{
  return (_method);
}

bool  HttpRequest::isComplete(void) const
{
  return (_isComplete);
}
