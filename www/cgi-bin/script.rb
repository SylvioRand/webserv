# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    script.rb                                          :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/09/14 16:22:54 by srandria          #+#    #+#              #
#    Updated: 2025/09/14 16:45:51 by srandria         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

#!/usr/bin/env ruby

require 'cgi'

cgi = CGI.new

counter = 0
if cgi.cookies['counter'] && !cgi.cookies['counter'].empty?
  counter = cgi.cookies['counter'][0].to_i
end

counter += 1

cookie = CGI::Cookie.new(
  'name' => 'counter',
  'value' => counter.to_s,
  'path' => '/',
  'expires' => Time.now + 60
)

puts cgi.header('type' => 'text/html', 'cookie' => [cookie])
puts "<!DOCTYPE html>"
puts "<html lang='fr'>"
puts "<head><meta charset='UTF-8'><title >Compteur avec Cookie</title></head>"
puts "<body>"
puts "<h1>Vous avez visité cette page #{counter} fois.</h1>"
puts "</body></html>"
