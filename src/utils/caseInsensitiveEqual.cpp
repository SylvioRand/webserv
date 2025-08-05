/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   caseInsensitiveEqual.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 10:35:49 by srandria          #+#    #+#             */
/*   Updated: 2025/08/05 10:37:15 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/utils/utils.hpp"

bool caseInsensitiveEqual(const std::string& a, const std::string& b)
{
    if (a.size() != b.size()) return false;
    for (std::string::const_iterator itA = a.begin(), itB = b.begin();
         itA != a.end(); ++itA, ++itB)
    {
        if (::tolower(*itA) != ::tolower(*itB))
            return false;
    }
    return true;
}
