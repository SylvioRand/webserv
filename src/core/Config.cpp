/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zramahaz <zramahaz@student.42antanana>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/26 14:43:58 by zramahaz          #+#    #+#             */
/*   Updated: 2025/09/09 19:01:55 by zramahaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/core/Config.hpp"
#include <cstdarg>
#include <cstddef>
#include <cstdlib>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>

std::string cleanBlock(const std::string& raw);

Config::Config(std::string filepath) : _config_path(filepath)
{
  // TODO Uncoment this line when parsing fixed
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
  std::cout << std::endl;
  std::cout << std::endl;
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
  result.host = "127.0.0.1";
  result.port = 8080;
  result.root = "./srandria";

  LocationConfig  loc;
  LocationConfig  loc2;
  loc.root = "./www/";
  loc.path = "/";
  loc.methods.push_back("GET, POST");
  loc.autoindex = false;
  loc2.root = "./www/uploads/";
  loc2.path = "/upload/";
  loc.error_pages[405] = "/405.html";
  result.locations["/"] = loc;
  /*
  result.locations["/upload"] = loc2;
  this->_servers.push_back(result);
  */

  ServerConfig  result2;
  result2.server_name = "localhost";
  result2.client_max_body_size = 10485760;
  result2.error_pages[404] = "/404.html";
  result2.host = "127.0.0.2";
  result2.port = 8080;
  result2.locations["/"] = loc;
  result2.locations["/"] = loc2;
  this->_servers.push_back(result2);

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
    /*
    if (s.client_max_body_size > MAX_BODY_LIMIT)
      return false;
      */
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

std::string                   Config::extractBlockContentServer(const std::string& block) {
    size_t start = block.find('{');
    size_t end = block.rfind('}');
    if (start == std::string::npos || end == std::string::npos || end <= start)
        throwWithLog(LOG_ERROR, "Invalid server block format");

    return block.substr(start + 1, end - start - 1);
}

bool  Config::blockServerIsValid(const std::string& input, size_t& braceStart, size_t& pos)
{
    if (braceStart == pos + 6) // server{
      return (true);
    else // server { ou server a{
    {
      std::istringstream iss(input.substr(pos));
      std::string     arg, key;
      iss >> key >> arg;
      if (arg.at(0) == '{') // server {
        return (true);
      return (false); // server arg{
    }
    return (false);
}

std::vector<std::string>      Config::extractServerBlocks(const std::string& input) {
    std::vector<std::string> blocks;
    size_t  pos = 0;
    size_t  i = 0;


    while ((pos = input.find("server", pos)) != std::string::npos) {
      // Vérifie que "server" est suivi d'une accolade
      size_t braceStart = input.find("{", pos);
      if (braceStart == std::string::npos)
      {
        throwWithLog(LOG_ERROR,
            "Expected '{' after 'server' at position " + toString(203));
      }
      if (pos != i || !this->blockServerIsValid(input, braceStart, pos))
        throwWithLog(LOG_ERROR, "Unknown directive");
 
      // Trouver la fin du bloc avec gestion des accolades imbriquéesint
      int depth = 1;
      i = braceStart + 1;
      while (i < input.size() && depth > 0) {
        if (input[i] == '{') depth++;
        else if (input[i] == '}') depth--;
          ++i;
      }

      if (depth != 0)
      {
        throwWithLog(LOG_ERROR, "Unmatched braces in server block starting at position " + toString(174));
      }

      blocks.push_back(input.substr(pos, i - pos));
      pos = i; // Continuer après ce blockServerIsValid
    }

    return blocks;
}

bool  Config::blockLocationIsValid(const std::string& content, size_t pos)
{
  std::istringstream iss(content.substr(pos));
  std::string        key, arg;
  iss >> key >> arg;
  if (key != "location"){ // locations ou locationnn ...
    return (false);
  }
  if (arg.at(0) == '{') // location {
    return (true);
  else if (arg.at(0) == '/') // location /...
  {
    std::string   brace;
    iss >> brace;
    if (brace.at(0) == '{') // location /... {
      return (true);
    return (false); // location /... a
  }
  return (false); // location a
}

std::vector<std::string>  Config::extractLocationBlocks(const std::string& content)
{
  std::vector<std::string> locations;
  size_t pos = 0;

  while ((pos = content.find("location", pos)) != std::string::npos)
  {
    // Vérifie que "location" est suivi d'une accolade
    size_t braceStart = content.find("{", pos);
    if (braceStart == std::string::npos) {
      throwWithLog(LOG_ERROR, "Expected '{' after 'server' at position " + toString(240));
    }
    if (!this->blockLocationIsValid(content, pos))
      throwWithLog(LOG_ERROR, "Unknown directive or argument invalid at position " + toString(261));

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


std::string Config::extractBlockContentLocation(const std::string &block, std::string &path)
{
  size_t start = block.find('{');
  size_t end = block.rfind('}');
  if (start == std::string::npos || end == std::string::npos || end <= start)
        throwWithLog(LOG_ERROR, "Invalid server block format");
  if (start != std::string::npos)
  {
    std::string loc = block.substr(0, start);
    std::istringstream  iss(loc);
    std::string key;

    iss >> key >> path;
  }
  return block.substr(start + 1, end - start - 1);
}


std::string Config::insertSpaceBeforeBrace(const std::string& line)
{
  std::string result;
  for (size_t i = 0; i < line.size(); ++i)
  {
    if (line[i] == '{')
    {
      // Si le caractère précédent n'est pas un espace
      if (i > 0 && line[i - 1] != ' ') {
        result += ' ';
      }
    }
    result += line[i];
  }
  return result;
}

void  Config::applyDirectiveTolocationConfig(const std::string& key, const std::vector<std::string>& value, LocationConfig& location_config)
{
  if (key == "path" && location_config.path.empty()) {
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": argument is invalid");
    location_config.path = value[0];
  }
  else if (key == "root" && location_config.root.empty()) {
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": argument is invalid");
    location_config.root = value[0];
  }
  // TODO : client_max_body_size n'est pas repetable
  else if (key == "client_max_body_size") {
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": argument is invalid");
    location_config.client_max_body_size = parseSize(key, value[0]);
  }
  else if (key == "upload_dir" && location_config.upload_dir.empty()) {
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": argument is invalid");
    location_config.upload_dir = value[0];
  }
  else if (key == "cgi_extension" && location_config.cgi_extension.empty()) {
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": argument is invalid");

    location_config.cgi_extension = value[0];
  }
  else if (key == "cgi_path" && location_config.cgi_path.empty()) {
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": argument is invalid");
    location_config.cgi_path = value[0];
  }
  else if (key == "return" && location_config.redirect.size() == 0) {
    if (value.size() == 0 || value.size() > 2)
      throwWithLog(LOG_ERROR, key + ": argument is invalid");
    int code = stringToInt(key, value[0]);
    if (value.size() == 1)
      location_config.redirect[code] = "";
    else {
      // TODO : verifier si la valeur de value[1] = "..." ou /... ou http(s)://
      location_config.redirect[code] = value[1];
    }
  }
  // TODO : autoindex n'est pas repetable
  else if (key == "autoindex") {
    if (value.size() == 0 || value.size() > 1 || (value[0] != "on" && value[0] != "off"))
      throwWithLog(LOG_ERROR, key + ": argument is invalid");
    if (value[0] == "on")
      location_config.autoindex = true;
    else
      location_config.autoindex = false;
  }
  else if (key == "index") {
    if (value.size() == 0)
      throwWithLog(LOG_ERROR, key + ": argument is invalid");
    location_config.indexs.insert(location_config.indexs.end(), value.begin(), value.end());

  }
  else if (key == "methods") {
    if (value.size() == 0)
      throwWithLog(LOG_ERROR, key + ": argument is invalid");
    location_config.methods = value;
  }
  else if (key == "error_page") {
    if (value.size() != 2)
        throwWithLog(LOG_ERROR, key + ": argument is invalid");
    int code = stringToInt(key, value[0]);
    location_config.error_pages[code] = value[1];
  }
  else {
    throwWithLog(LOG_ERROR, "Unknown directive '" + key + "' in location block");
  }
}

void  Config::appendHeritedDirective(ServerConfig &config,
    LocationConfig &location_config)
{
  if (location_config.root.empty()) location_config.root = config.root;
  if (location_config.indexs.size() == 0) location_config.indexs = config.indexs;
  if (location_config.upload_dir.empty()) location_config.upload_dir = config.upload_dir;
}

void                          Config::parseLocationBlocks(std::string &block, ServerConfig &config)
{
  LocationConfig  location_config;

  block = insertSpaceBeforeBrace(block);
  block = extractBlockContentLocation(block, location_config.path);
  
  std::istringstream contentStream(block);
  std::string directive;

  location_config.client_max_body_size = config.client_max_body_size;
  location_config.error_pages = config.error_pages;
  location_config.autoindex = config.autoindex;

  while (std::getline(contentStream, directive, ';')) {
    directive = trim(directive);
        
    std::istringstream lineStream(directive);
    std::string key;
    lineStream >> key;

    if (key.empty())
      continue;

    std::vector<std::string> value;
    std::string token;
    while (lineStream >> token)
      value.push_back(token);


    applyDirectiveTolocationConfig(key, value, location_config);
  }

  appendHeritedDirective(config, location_config);

  if (location_config.path.empty())
    throwWithLog(LOG_ERROR, "path of location is empty");
  config.locations[location_config.path] = location_config;
}

void                          Config::applyDirectiveToServerConfig(const std::string& key, std::vector<std::string>& value, ServerConfig& config) {
  if (key == "listen" && config.host.empty()) {
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": argument is invalid");
    size_t colon = value[0].find(":");
    std::string portStr;
    if (colon == std::string::npos){
      if (value[0].find(".") != std::string::npos)
        throwWithLog(LOG_ERROR, key + " Invalid port format: " + value[0]);
      config.host = "0.0.0.0";
      portStr = value[0];
    }
    else {
      config.host = value[0].substr(0, colon);
      portStr = value[0].substr(colon + 1);
    }
    if (portStr.empty())
      throwWithLog(LOG_ERROR, key + " Invalid listen format: " + value[0]);
    config.port = stringToInt("port", portStr); // C++98-compatible stoi
  }
  else if (key == "server_name" && config.server_name.empty()) {
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": argument is invalid");
    config.server_name = value[0];
  }
  else if (key == "root" && config.root.empty()) {
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": argument is invalid");
    config.root = value[0];
  }
  else if (key == "index") {
    if (value.size() == 0)
      throwWithLog(LOG_ERROR, key + ": argument is invalid");
    config.indexs.insert(config.indexs.end(), value.begin(), value.end());
  }
  else if (key == "upload_dir" && config.upload_dir.empty()) {
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": argument is invalid");
    config.upload_dir = value[0];
  }

  // TODO : autoindex n'est pas repetable
  else if (key == "autoindex") {
    if (value.size() == 0 || value.size() > 1 || (value[0] != "on" && value[0] != "off"))
      throwWithLog(LOG_ERROR, key + ": argument is invalid");
    if (value[0] == "on")
      config.autoindex = true;
    else
      config.autoindex = false;
  }
  else if (key == "error_page") {
    if (value.size() != 2)
      throwWithLog(LOG_ERROR, key + ": argument is invalid");
    int code = stringToInt(key, value[0]);
    config.error_pages[code] = value[1];
  }
  // TODO : client_max_body_size n'est pas repetable
  else if (key == "client_max_body_size") {
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": argument is invalid");
    config.client_max_body_size = parseSize(key, value[0]); // gère les suffixes M, K
  }
  else if (key == "return" && config.redirect.size() == 0) {
    if (value.size() == 0 || value.size() > 2)
      throwWithLog(LOG_ERROR, key + ": argument is invalid");
    int code = stringToInt(key, value[0]);
    if (value.size() == 1)
      config.redirect[code] = "";
    else {
      // TODO : verifier si la valeur de value[1] = "..." ou /... ou http(s)://
      config.redirect[code] = value[1];
    }
  }
  else {
    throwWithLog(LOG_ERROR, "Unknown directive '" + key + "' in server block or there is a duplication");
  }
}

void                          Config::setDirectiveToServerConfig(ServerConfig& config)
{
  if (config.host.empty()) {
    config.host = "0.0.0.0";
    config.port = 8081;
  }
  if (config.indexs.size() == 0) {
    config.indexs.push_back("index.html");
  }
}

void                          Config::parseDirectivesIntoConfig(const std::string& block, ServerConfig& config) {
  std::istringstream contentStream(block);
  std::string directive;

  config.client_max_body_size = 1048576;
  config.autoindex = false;

  while (std::getline(contentStream, directive, ';')) {
    directive = trim(directive);
        
    std::istringstream lineStream(directive);
    std::string key;
    lineStream >> key;

    if (key.empty())
      continue;

    std::vector<std::string> value;
    std::string token;
    while (lineStream >> token)
      value.push_back(token);
    
    applyDirectiveToServerConfig(key, value, config);

  }
  setDirectiveToServerConfig(config);
}

void                          Config::createLocationDefautl(ServerConfig& config)
{
  LocationConfig  location_config;
  
  location_config.path = "/";
  location_config.root = config.root;
  location_config.indexs = config.indexs;
  location_config.upload_dir = config.upload_dir;
  location_config.client_max_body_size = config.client_max_body_size;
  location_config.error_pages = config.error_pages;
  location_config.autoindex = config.autoindex;
  location_config.methods.push_back("GET");


  config.locations[location_config.path] = location_config;
}

void                          Config::parseServerBlock_(std::string &content)
{
  ServerConfig config;
  content = extractBlockContentServer(content);
  
  std::cout << "content = |" + content + "|" << std::endl;

  // Extraire les blocs location
  std::vector<std::string> locationBlocks = extractLocationBlocks(content);

  // Supprimer les blocs location du contenu principal
  for (std::vector<std::string>::const_iterator it = locationBlocks.begin(); it != locationBlocks.end(); ++it) {
    size_t pos = content.find(*it);
    if (pos != std::string::npos) {
      content.erase(pos, it->length());
    }
  }

  // parse des directives
  parseDirectivesIntoConfig(content, config);
  
  // parse des locations
  for (std::vector<std::string>::iterator it = locationBlocks.begin(); it != locationBlocks.end(); ++it){
    parseLocationBlocks(*it, config);
  }
  if (config.locations.find("/") == config.locations.end())
    createLocationDefautl(config);

  this->_servers.push_back(config);
}



std::string                   Config::trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\r\n");
    size_t last = str.find_last_not_of(" \t\r\n");
    if (first == std::string::npos || last == std::string::npos)
        return "";
    return str.substr(first, last - first + 1);
}


int                           Config::stringToInt(const std::string& key, const std::string& str)
{
    char *endptr = NULL;
    long value = std::strtol(str.c_str(), &endptr, 10);

    if (*endptr == '\0' && value >= 0) {
      std::istringstream iss(str);
      int result = 0;
      iss >> result;
      return result;
    }
    else
      throwWithLog(LOG_FATAL, \
          "[emerg] invalid " + key + " " + str + " in /etc/nginx/nginx.conf:45");
    return (0);
}


size_t                        Config::parseSize(const std::string& key, const std::string& str)
{
    char suffix = str[str.size() - 1];
    std::string number = str;

    if (suffix == 'M' || suffix == 'M' || \
          suffix == 'K' || suffix == 'k' || \
            suffix == 'G' || suffix == 'g') {
        number = str.substr(0, str.size() - 1);
    }

    size_t base = stringToInt(key, number);
    if (suffix == 'G' || suffix == 'g') return (base * 1024 * 1024 * 1024);
    if (suffix == 'M' || suffix == 'm') return (base * 1024 * 1024);
    if (suffix == 'K' || suffix == 'k') return (base * 1024);
    return base;
}



void                          Config::printServers() const {
    for (size_t i = 0; i < this->_servers.size(); ++i) {
        std::cout << std::endl << std::endl;
        const ServerConfig& config = this->_servers[i];

        std::cout << "server[" << i << "].host = |" << config.host << "|" << std::endl;
        std::cout << "server[" << i << "].port = |" << config.port << "|" << std::endl;
        std::cout << "server[" << i << "].root = |" << config.root << "|" << std::endl;

        std::cout << "server[" << i << "].index = |";
        for (size_t j = 0; j < config.indexs.size(); ++j) {
            std::cout << config.indexs[j];
            if (j + 1 < config.indexs.size()) std::cout << ", ";
        }
        std::cout << "|" << std::endl;

        for (std::map<int, std::string>::const_iterator it = config.redirect.begin(); it != config.redirect.end(); ++it)
          std::cout << "server[" << i << "].redirect[" << it->first << "] = |" << it->second << "|" << std::endl;

        std::cout << "server[" << i << "].upload_dir = |" << config.upload_dir << "|" << std::endl;
        std::cout << "server[" << i << "].server_name = |" << config.server_name << "|" << std::endl;
        std::cout << "server[" << i << "].autoindex = |" << (config.autoindex ? "true" : "false") << "|" << std::endl;
        std::cout << "server[" << i << "].client_max_body_size = |" << config.client_max_body_size << "|" << std::endl;

        for (std::map<int, std::string>::const_iterator ep = config.error_pages.begin(); ep != config.error_pages.end(); ++ep) {
            std::cout << "server[" << i << "].error_pages[" << ep->first << "] = |" << ep->second << "|" << std::endl;
        }

        for (std::map<std::string, LocationConfig>::const_iterator loc = config.locations.begin(); loc != config.locations.end(); ++loc) {
            const LocationConfig& lcfg = loc->second;
            std::string path = loc->first;

            std::cout << "server[" << i << "].locations[" << path << "].path = |" << lcfg.path << "|" << std::endl;
            std::cout << "server[" << i << "].locations[" << path << "].root = |" << lcfg.root << "|" << std::endl;

            std::cout << "server[" << i << "].locations[" << path << "].index = |";
            for (size_t j = 0; j < lcfg.indexs.size(); ++j) {
                std::cout << lcfg.indexs[j];
                if (j + 1 < lcfg.indexs.size()) std::cout << ", ";
            }
            std::cout << "|" << std::endl;

            for (std::map<int, std::string>::const_iterator ep = lcfg.redirect.begin(); ep != lcfg.redirect.end(); ++ep) {
                std::cout << "server[" << i << "].locations[" << path << "].redirect[" << ep->first << "] = |" << ep->second << "|" << std::endl;
            }

            std::cout << "server[" << i << "].locations[" << path << "].methods = |";
            for (size_t j = 0; j < lcfg.methods.size(); ++j) {
                std::cout << lcfg.methods[j];
                if (j + 1 < lcfg.methods.size()) std::cout << ", ";
            }
            std::cout << "|" << std::endl;

            std::cout << "server[" << i << "].locations[" << path << "].autoindex = |" << (lcfg.autoindex ? "true" : "false") << "|" << std::endl;
            std::cout << "server[" << i << "].locations[" << path << "].upload_dir = |" << lcfg.upload_dir << "|" << std::endl;
            std::cout << "server[" << i << "].locations[" << path << "].client_max_body_size = |" << lcfg.client_max_body_size << "|" << std::endl;
            std::cout << "server[" << i << "].locations[" << path << "].cgi_extension = |" << lcfg.cgi_extension << "|" << std::endl;
            std::cout << "server[" << i << "].locations[" << path << "].cgi_path = |" << lcfg.cgi_path << "|" << std::endl;

            for (std::map<int, std::string>::const_iterator ep = lcfg.error_pages.begin(); ep != lcfg.error_pages.end(); ++ep) {
                std::cout << "server[" << i << "].locations[" << path << "].error_pages[" << ep->first << "] = |" << ep->second << "|" << std::endl;
            }
        }

        std::cout << std::endl << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    }
}
