#!/usr/bin/env ruby
require 'cgi'

cgi = CGI.new

# --- Gestion du compteur via cookie ---
counter = 0
if cgi.cookies['counter'] && !cgi.cookies['counter'].empty?
  counter = cgi.cookies['counter'][0].to_i
end
counter += 1
cookie = CGI::Cookie.new('name' => 'counter', 'value' => counter.to_s, 'path' => '/')

# --- Récupération des paramètres GET/POST ---
params = cgi.params.map { |k, v| "#{k}=#{v.join(',')}" }.join(", ")
params = "Aucun paramètre" if params.empty?

# --- Corps HTML ---
body = <<~HTML
  <!DOCTYPE html>
  <html lang="fr">
  <head>
    <meta charset="UTF-8">
    <title>CGI Ruby Demo</title>
  </head>
  <body>
    <h1>CGI Ruby en action 🚀</h1>
    <p><strong>Méthode :</strong> #{ENV['REQUEST_METHOD']}</p>
    <p><strong>Paramètres :</strong> #{params}</p>
    <p><strong>Vous avez visité cette page #{counter} fois.</strong></p>
  </body>
  </html>
HTML

# --- En-têtes + body ---
puts "Content-Type: text/html"
puts "Set-Cookie: #{cookie}"
puts "Content-Length: #{body.bytesize}"
puts
puts body
