/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   timeouts.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 10:26:28 by srandria          #+#    #+#             */
/*   Updated: 2025/09/14 10:24:02 by srandria         ###   ########.fr       */
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
}

#endif
