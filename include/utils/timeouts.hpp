/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   timeouts.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 10:26:28 by srandria          #+#    #+#             */
/*   Updated: 2025/09/13 16:42:45 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TIMEOUTS_HPP
#define TIMEOUTS_HPP

// timeout en seconde
namespace timeouts {
    const int CLIENT_HEADER_TIMEOUT = 5;
    const int CLIENT_BODY_TIMEOUT   = 10;
    const int SEND_TIMEOUT          = 10;
    const int KEEPALIVE_TIMEOUT     = 15;
    const int PROXY_CONNECT_TIMEOUT = 3;
    const int PROXY_READ_TIMEOUT    = 15;
    const int PROXY_SEND_TIMEOUT    = 5;
}

#endif
