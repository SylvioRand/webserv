/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zramahaz <zramahaz@student.42antanana>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/26 14:43:58 by zramahaz          #+#    #+#             */
/*   Updated: 2025/08/04 09:25:32 by zramahaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/Config.hpp"

Config::Config(std::string filepath) : _config_path(filepath)
{
  this->load_();
  // Skip this if the config has been parsed earlier
  this->createServerConfigManually();
  this->isValid_();
}

Config::~Config(void)
{ }

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
  std::string containt;
  size_t      pos;

  _config_file.open(_config_path.c_str());
  if (!_config_file.is_open()) {
    throwWithLog(LOG_ERROR, "Failed to open config file: " + _config_path);
  }
  
  while (std::getline(_config_file, _current_line))
  { _line_number++; this->skipWhiteSpace_(); if (_current_line.empty() || _current_line[0] == '#')
    {
      continue;
    }
    pos = _current_line.find("#");
    if (pos != std::string::npos)
    {
      _current_line = _current_line.substr(0, pos);
    }
    containt += _current_line;
  }

  std::cout << containt << std::endl;
  std::cout << "-------------------------" << std::endl;
  this->parseServerBlock_(containt);    // parse de zramahaz
  
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

// STATIC FUNCTION
std::string Config::trim(const std::string& str) {
  size_t first = str.find_first_not_of(" \t");
  if (first == std::string::npos)
  return ""; // La chaîne est vide ou remplie d'espaces
  
  size_t last = str.find_last_not_of(" \t");
  return str.substr(first, last - first + 1);
}



// TODO : This function serves as the entry point for the configuration file parser.
// Don`t forget comment is allowed too on the server bloc of the file configuration

int Config::checkKeyAndAddValue_(std::string &token, int i, std::string &containtServerBlock)
{
  std::istringstream iss(token);
  std::string        key;
  std::string        value;

  (void)containtServerBlock;
  iss >> key;
  if (key == "listen")
  {
    std::cout << "---------key = " << key << std::endl;
    containtServerBlock = containtServerBlock.substr(containtServerBlock.find(";") + 1);
    return (1);
  }
  else if (key == "index")
  {
    std::cout << "---------key = " << key << std::endl;
    containtServerBlock = containtServerBlock.substr(containtServerBlock.find(";") + 1);
    iss >> value;
    this->_servers[i].index = value;
    return (1);
  }
  else if (key == "server_name")
  {
    std::cout << "---------key = " << key << std::endl;
    containtServerBlock = containtServerBlock.substr(containtServerBlock.find(";") + 1);
    iss >> value;
    this->_servers[i].index = value; 
    return (1);
  }
  else if (key == "root")
  {
    std::cout << "---------key = " << key << std::endl;
    containtServerBlock = containtServerBlock.substr(containtServerBlock.find(";") + 1);
    iss >> value;
    this->_servers[i].index = value; 
    return (1);
  }
  else if (key == "error_page")
  {
    std::cout << "---------key = " << key << std::endl;
    containtServerBlock = containtServerBlock.substr(containtServerBlock.find(";") + 1);
    return (1);
  }
  else if (key == "client_max_body_size")
  {
    std::cout << "-----------key = " << key << std::endl;
    containtServerBlock = containtServerBlock.substr(containtServerBlock.find(";") + 1);
    return (1);
  }
  else if (key == "location")
  {
    std::cout << "---------key = " << key << std::endl;
    containtServerBlock = containtServerBlock.substr(containtServerBlock.find(" ") + 1);
    return (0);
  }
  return (1);
}

int Config::parseLocation_(std::string containtServerBlock, int i)
{
  (void)i;
  std::cout << containtServerBlock << std::endl;
  std::istringstream iss(containtServerBlock);
  std::string        path;

  iss >> path;
  if (path[0] != '/')
    throwWithLog(LOG_ERROR, "location path error");
  
  return (1);
}

void  Config::parseBlock_(std::string containtServerBlock, int i)
{
  int                 status;
  std::istringstream  iss(containtServerBlock);
  std::string         token;

  while (std::getline(iss, token, ';')) {
    std::cout << "|" << token << "|" << std::endl;
    status = checkKeyAndAddValue_(token, i, containtServerBlock);
    if (status == -1)
      throwWithLog(LOG_ERROR, "directive error");
    else if (status == 0)
    {
      parseLocation_(containtServerBlock, i);
    }
  }


  // std::cout << containtServerBlock << std::endl;
  std::cout << std::endl;
}

void Config::parseServerBlock_(std::string &containt)
{
  size_t      pos = 0;
  int         i = 0;
  std::string containtServerBlock;

  pos = containt.find("server{");
  while (pos != std::string::npos)
  {
    this->_servers.push_back(ServerConfig());
    containtServerBlock = containt.substr(pos + 7);
    containt = containt.substr(pos + 7);
    pos = containt.find("server{");
    if (pos != std::string::npos)
    {
      containtServerBlock = containtServerBlock.substr(0, pos);
    }
    parseBlock_(containtServerBlock, i);
    ++i;
  }
}