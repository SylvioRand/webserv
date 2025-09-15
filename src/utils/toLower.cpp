/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   toLower.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 09:10:41 by srandria          #+#    #+#             */
/*   Updated: 2025/08/15 09:11:30 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/utils/utils.hpp"

std::string toLower(const std::string &s)
{
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   (int(*)(int))std::tolower);
    return (result);
}
