/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zramahaz <zramahaz@student.42antanana>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/26 14:43:58 by zramahaz          #+#    #+#             */
/*   Updated: 2025/07/28 17:16:21 by zramahaz         ###   ########.fr       */
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
  _config_file.open(_config_path.c_str());
  if (!_config_file.is_open()) {
    throwWithLog(LOG_ERROR, "Failed to open config file: " + _config_path);
  }
  
  while (std::getline(_config_file, _current_line))
  { _line_number++; this->skipWhiteSpace_(); if (_current_line.empty() || _current_line[0] == '#')
    {
      continue;
    }
    
    if (_current_line.find("server") == 0)
    {
      this->parseServerBlock_();    // parse de zramahaz
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

int Config::countOccurrence(const std::string& chaine, char c) {
  return std::count(chaine.begin(), chaine.end(), c);
}


// TODO : This function serves as the entry point for the configuration file parser.
// Don`t forget comment is allowed too on the server bloc of the file configuration


// fonction qui cherche "server {"
bool  Config::findServerBrace_(void)
{
  size_t brace_pos = _current_line.find("{");
  
  // Si "server" et "{" sont dans la meme ligne
  if (brace_pos != std::string::npos)
  { std::string before_brace = _current_line.substr(0, brace_pos);
    std::string after_brace = _current_line.substr(brace_pos + 1);
    std::cout << "before_brace: " << "|" << before_brace << "|" << std::endl;
    std::cout << "after_brace: " << "|" << after_brace << "|" << std::endl;
    if (trim(before_brace) == "server")
    {
      return (true);
    }
    return (false);
  } else if (trim(_current_line) == "server")
  {
    while (std::getline(_config_file, _current_line))
    { _line_number++; this->skipWhiteSpace_(); if (_current_line.empty() || _current_line[0] == '#')
      {
        continue;
      }
      if (_current_line.find("{") != std::string::npos)
      {
        return (true);
      } else
        return (false);
    }
  }
  return (false);
}


void  Config::addStringValue_(std::istringstream &iss, int id)
{
  if (id == 1)
  {
    iss >> this->_servers[0].server_name;
  }
  else if (id == 2)
  {
    iss >> this->_servers[0].root;
  }
  else if (id == 3)
  {
    iss >> this->_servers[0].index;
  }
}

void  Config::appendValueDirective_(std::string &token)
{
  int i = 0;
  std::istringstream iss(token);
  std::string key;

  iss >> key;
  std::string keys[6] = {"listen", "server_name", "root", "index", "error_page", "client_max_body_size"};
  for (i = 0; i < 6; i++) {
    if (keys[i] == key)
      break ;
  }
  switch (i)
  {
    case 0:
      /* code */
      break;
    case 1:
      addStringValue_(iss, 1);
      break;
    case 2:
      addStringValue_(iss, 2);
      break;
    case 3:
      addStringValue_(iss, 3);
      break;
    case 4:
      /* code */
      break;
    case 5:
      /* code */
      break;
    
    default:
      throwWithLog(LOG_ERROR, "mot cle inconnu");
  }
}


void  Config::parseDirective_(void)
{
  std::istringstream iss(_current_line);
  std::string token;
    
  while (std::getline(iss, token, ';')) {
    std::cout << "|" << token << "|" << std::endl;
    appendValueDirective_(token);
    }
}

void  Config::parseDirectiveAndBloc_(void)
{

  std::cout << "debut dans le bloc, directive" << std::endl;
  std::cout << "|" << _current_line << "|" << std::endl;
  while (std::getline(_config_file, _current_line))
  { _line_number++; this->skipWhiteSpace_(); if (_current_line.empty() || _current_line[0] == '#')
    {
      continue;
    }
    parseDirective_();
  }
  std::cout << "fin dans le bloc, directive" << std::endl;
}

void Config::parseServerBlock_(void)
{
  this->_servers.push_back(ServerConfig());

  if (!findServerBrace_())
    throwWithLog(LOG_ERROR, "Error Parsing");
  parseDirectiveAndBloc_();

}