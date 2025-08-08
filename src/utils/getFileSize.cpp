/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   getFileSize.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/08 09:00:05 by srandria          #+#    #+#             */
/*   Updated: 2025/08/08 09:00:46 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/utils/utils.hpp"

off_t getFileSize(const std::string& path)
{
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) == 0)
        return statbuf.st_size;
    return -1;
}
