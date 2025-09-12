#!/usr/bin/env ruby

require 'cgi'

cgi = CGI.new

# Récupérer le cookie "counter"
counter = 0
if cgi.cookies['counter'] && !cgi.cookies['counter'].empty?
  counter = cgi.cookies['counter'][0].to_i
end

# Incrémenter le compteur
counter += 1

# Créer/mettre à jour le cookie
cookie = CGI::Cookie.new(
  'name' => 'counter',
  'value' => counter.to_s,
  'path' => '/',
  'expires' => Time.now + 60
)

# Générer la réponse HTTP
puts cgi.header('type' => 'text/html', 'cookie' => [cookie])
puts "<!DOCTYPE html>"
puts "<html lang='fr'>"
puts "<head><meta charset='UTF-8'><title>Compteur avec Cookie</title></head>"
puts "<body>"
puts "<h1>Vous avez visité cette page #{counter} fois.</h1>"
puts "</body></html>"
