/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   generateTempFileName.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/19 11:39:12 by srandria          #+#    #+#             */
/*   Updated: 2025/08/19 11:40:33 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/utils/utils.hpp"

std::string generateTempFilename()
{
    static int counter = 0; // compteur pour distinguer plusieurs fichiers créés au même instant
    std::ostringstream ss;
    ss << "/tmp/upload_" << std::time(nullptr) << "_" << counter++ << ".bin";
    return ss.str();
}
