/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zramahaz <zramahaz@student.42antanana>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/26 14:43:58 by zramahaz          #+#    #+#             */
/*   Updated: 2025/08/05 14:34:37 by zramahaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/Config.hpp"

std::string cleanBlock(const std::string& raw);

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
  std::string content;
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
    content += _current_line;
  }
  
  // parse de zramahaz
  std::vector<std::string> serverBlocks = extractServerBlocks(content);
  for (size_t i = 0; i < serverBlocks.size(); ++i) {
    this->parseServerBlock_(serverBlocks[i]);
  }
  printServers();

  // Uncomment this block once the configuration file parsing has been parsed.
  /*
  if (this->_servers.empty()) {
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


// TODO : This function serves as the entry point for the configuration file parser.
// Don`t forget comment is allowed too on the server bloc of the file configuration

std::string Config::insertSpaceBeforeBrace(const std::string& line) {
    std::string result;
    for (size_t i = 0; i < line.size(); ++i) {
        if (line[i] == '{' && i > 0 && !isspace(line[i - 1])) {
            result += ' ';
        }
        result += line[i];
    }
    return result;
}

std::string Config::extractBlockContent(const std::string& block) {
    size_t start = block.find('{');
    size_t end = block.rfind('}');
    if (start == std::string::npos || end == std::string::npos || end <= start)
        throwWithLog(LOG_ERROR, "Invalid server block format");

    return block.substr(start + 1, end - start - 1);
}

std::vector<std::string> Config::extractServerBlocks(const std::string& input) {
    std::vector<std::string> blocks;
    size_t pos = 0;

    while ((pos = input.find("server", pos)) != std::string::npos) {
        // Vérifie que "server" est suivi d'une accolade
        size_t braceStart = input.find("{", pos);
        if (braceStart == std::string::npos) {
            throwWithLog(LOG_ERROR, "Expected '{' after 'server' at position " + 161);
        }

        // Trouver la fin du bloc avec gestion des accolades imbriquées
        int depth = 1;
        size_t i = braceStart + 1;
        while (i < input.size() && depth > 0) {
            if (input[i] == '{') depth++;
            else if (input[i] == '}') depth--;
            ++i;
        }

        if (depth != 0) {
            throwWithLog(LOG_ERROR, "Unmatched braces in server block starting at position " + 174);
        }

        blocks.push_back(input.substr(pos, i - pos));
        pos = i; // Continuer après ce bloc
    }

    return blocks;
}

std::vector<std::string> Config::extractLocationBlocks(const std::string& content) {
    std::vector<std::string> locations;
    size_t pos = 0;

    while ((pos = content.find("location", pos)) != std::string::npos) {
        size_t braceStart = content.find("{", pos);
        if (braceStart == std::string::npos) break;

        int depth = 1;
        size_t i = braceStart + 1;
        while (i < content.size() && depth > 0) {
            if (content[i] == '{') depth++;
            else if (content[i] == '}') depth--;
            ++i;
        }

        if (depth != 0) {
            throwWithLog(LOG_ERROR, "Unmatched braces in location block");
        }

        locations.push_back(content.substr(pos, i - pos));
        pos = i;
    }

    return locations;
}

void Config::parseServerBlock_(std::string &content)
{
  ServerConfig config;
  content = extractBlockContent(content);
  
  // Extraire les blocs location
  std::vector<std::string> locationBlocks = extractLocationBlocks(content);

  // Supprimer les blocs location du contenu principal
  for (std::vector<std::string>::const_iterator it = locationBlocks.begin(); it != locationBlocks.end(); ++it) {
    size_t pos = content.find(*it);
    if (pos != std::string::npos) {
      content.erase(pos, it->length());
    }
  }

  parseDirectivesIntoConfig(content, config);
  this->_servers.push_back(config);
}

void Config::parseDirectivesIntoConfig(const std::string& block, ServerConfig& config) {
  std::istringstream contentStream(block);
  std::string directive;

  while (std::getline(contentStream, directive, ';')) {
    directive = trim(directive);
        
    std::istringstream lineStream(directive);
    std::string key;
    lineStream >> key;

    std::string value;
    std::getline(lineStream, value);
    value = trim(value);

    applyDirectiveToServerConfig(key, value, config);
  }
}

void Config::applyDirectiveToServerConfig(const std::string& key, const std::string& value, ServerConfig& config) {
  if (key == "listen") {
    size_t colon = value.find(":");
    if (colon == std::string::npos)
        throwWithLog(LOG_ERROR, "Invalid listen format: " + value);
    config.host = value.substr(0, colon);
    std::string portStr = value.substr(colon + 1);
    config.port = stringToInt(portStr); // C++98-compatible stoi
  }
  else if (key == "server_name") {
    config.server_name = value;
  }
  else if (key == "root") {
    config.root = value;
  }
  else if (key == "index") {
    std::istringstream iss(value);
    std::string token;
    while (iss >> token) {
        config.index.push_back(token);
    }
  }
  else if (key == "error_page") {
    std::istringstream iss(value);
    std::string codeStr, path;
    iss >> codeStr >> path;
    int code = stringToInt(codeStr);
    config.error_pages[code] = path;
  }
  else if (key == "client_max_body_size") {
    config.client_max_body_size = parseSize(value); // gère les suffixes M, K
  }
  else if (key == "path") {
    config.path = value;
  }
  else {
    throwWithLog(LOG_ERROR, "Unknown directive '" + key + "' in server block");
  }
}

std::string Config::trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\r\n");
    size_t last = str.find_last_not_of(" \t\r\n");
    if (first == std::string::npos || last == std::string::npos)
        return "";
    return str.substr(first, last - first + 1);
}

int     Config::stringToInt(const std::string& str)
{
    std::istringstream iss(str);
    int result = 0;
    iss >> result;
    return result;
}

size_t  Config::parseSize(const std::string& str)
{
    if (str.empty()) return 0;
    char suffix = str[str.size() - 1];
    std::string number = str;

    if (suffix == 'M' || suffix == 'K') {
        number = str.substr(0, str.size() - 1);
    }

    size_t base = stringToInt(number);
    if (suffix == 'M') return base * 1024 * 1024;
    if (suffix == 'K') return base * 1024;
    return base;
}

void Config::printServers() const {
    for (size_t i = 0; i < this->_servers.size(); ++i) {
        const ServerConfig& config = this->_servers[i];
        std::cout << "server[" << i << "].host = |" << config.host << "|" << std::endl;
        std::cout << "server[" << i << "].port = |" << config.port << "|" << std::endl;
        std::cout << "server[" << i << "].server_name = |" << config.server_name << "|" << std::endl;
        std::cout << "server[" << i << "].root = |" << config.root << "|" << std::endl;
        std::cout << "server[" << i << "].path = |" << config.path << "|" << std::endl;

        std::cout << "server[" << i << "].index = |";
        for (size_t j = 0; j < config.index.size(); ++j) {
            std::cout << config.index[j];
            if (j + 1 < config.index.size()) std::cout << ", ";
        }
        std::cout << "|" << std::endl;

        std::cout << "server[" << i << "].client_max_body_size = |" << config.client_max_body_size << "|" << std::endl;

        for (std::map<int, std::string>::const_iterator it = config.error_pages.begin(); it != config.error_pages.end(); ++it) {
            std::cout << "server[" << i << "].error_pages[" << it->first << "] = |" << it->second << "|" << std::endl;
        }

        for (std::map<std::string, LocationConfig>::const_iterator it = config.locations.begin(); it != config.locations.end(); ++it) {
            std::cout << "server[" << i << "].locations[" << it->first << "] = |LocationConfig|" << std::endl;
            // Tu peux afficher les détails de LocationConfig ici si nécessaire
        }

        std::cout << "----------------------------------------" << std::endl;
    }
}

