/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   toUpper.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 09:38:00 by srandria          #+#    #+#             */
/*   Updated: 2025/08/15 09:38:49 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/utils/utils.hpp"

std::string toUpper(const std::string &s)
{
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   (int(*)(int))std::toupper);
    return result;
}
