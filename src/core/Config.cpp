/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */ /*                                                +#+#+#+#+#+   +#+           */ /*   Created: 2025/07/16 10:50:03 by srandria          #+#    #+#             */
/*   Updated: 2025/07/28 16:37:24 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/Config.hpp"

Config::Config(std::string filepath) : _config_path(filepath)
{
  this->load_();
  // Skip this if the config has been parsed earlier
  this->createServerConfigManually();
  /*
  if (!this->isValid_())
    throwWithLog(LOG_ERROR, "invalid server configuration");
    */
}

Config::~Config(void)
{

}

void Config::skipWhiteSpace_(void)
{
  // 1. Recherche du premier caractère qui n'est pas un espace ou tabulation
  size_t pos = _current_line.find_first_not_of(" \t");
  
  // 2. Vérification si un caractère non-blanc a été trouvé
  if (pos != std::string::npos) {
    // 3a. Si oui, on garde seulement la partie de la ligne après les espaces
    _current_line = _current_line.substr(pos);
  } else {
    // 3b. Si non (ligne vide ou que des espaces), on vide la ligne
    _current_line.clear();
  }
}

void Config::load_(void)
{
  _config_file.open(_config_path.c_str());
  if (!_config_file.is_open()) {
    throwWithLog(LOG_ERROR, "Failed to open config file: " + _config_path);
  }

  while (std::getline(_config_file, _current_line))
  {
    _line_number++;
    this->skipWhiteSpace_();
    if (_current_line.empty() || _current_line[0] == '#')
    {
        continue;
    }

    if (_current_line.find("server") == 0)
    {
        this->parseServerBlock_();

    }
  }

  // Uncomment this block once the configuration file parsing has been parsed.
  /*
  if (_servers.empty()) {
    throwWithLog(LOG_ERROR, "No server blocks found in config file");
  }
  */
}


// you can use this function to add manually a serverconfig without parsing
void  Config::createServerConfigManually(void)
{
  logger(LOG_INFO, "Creating server config manually");
  ServerConfig  result;
  result.server_name = "localhost";
  result.client_max_body_size = 10485760;
  result.error_pages[404] = "/404.html";
  result.host = "127.0.0.3";
  result.port = 8080;
  result.root = "./srandria";
  LocationConfig  loc;
  LocationConfig  loc2;
  result.locations["/"] = loc;
  result.locations["/upload"] = loc2;
  this->_servers.push_back(result);

  ServerConfig  result2;
  result.server_name = "localhost";
  result.client_max_body_size = 10485760;
  result.error_pages[404] = "/404.html";
  result.host = "127.0.0.4";
  result.port = 8081;
  this->_servers.push_back(result);

}


const std::vector<ServerConfig>& Config::getServers(void) const
{
  return (_servers);
}

bool Config::isValid_(void) const
{
  for (size_t i = 0; i < _servers.size(); ++i) {
    const ServerConfig& s = _servers[i];
    
    // Vérifie que chaque serveur a au moins un port d'écoute
    if (s.port < 1 || s.port > 65535)
      return false;
    
    // Vérifie les tailles maximales de body
    if (s.client_max_body_size > MAX_BODY_LIMIT)
      return false;
    
    // Vérifie qu`on a bien location /`
    if (s.locations.find("/") == s.locations.end())
      return false;
  }
  return true;
}



/* Zramahaz’s implementation starts here.     */

// TODO : This function serves as the entry point for the configuration file parser.
// Don`t forget comment is allowed too on the server bloc of the file configuration
void Config::parseServerBlock_(void)
{

}
