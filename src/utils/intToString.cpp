/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   intToString.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/29 16:47:46 by srandria          #+#    #+#             */
/*   Updated: 2025/07/29 16:48:01 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sstream>
#include <string>

std::string intToString(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}
