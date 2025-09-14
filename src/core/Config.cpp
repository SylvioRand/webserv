/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zramahaz <zramahaz@student.42antanana>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/26 14:43:58 by zramahaz          #+#    #+#             */
/*   Updated: 2025/09/14 10:52:38 by srandria         ###   ########.fr       */
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
#include <strings.h>

std::string cleanBlock(const std::string& raw);

Config::Config(std::string filepath) : _config_path(filepath)
{
  // TODO Uncoment this line when parsing fixed
  this->load_();
  // Skip this if the config has been parsed earlier
  // this->createServerConfigManually();
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


void  Config::load_(void)
{
  std::string buffer;
  size_t      pos;

  _config_file.open(_config_path.c_str());
  if (!_config_file.is_open())
    throwWithLog(LOG_ERROR, "Failed to open config file: " + _config_path + \
        "at position " + toString(__LINE__)); 

  while (std::getline(_config_file, _current_line))
  {
    _line_number++;
    skipWhiteSpace_();
    if (_current_line.empty() || _current_line[0] == '#')
      continue;
    pos = _current_line.find("#");
    if (pos != std::string::npos)
      _current_line = _current_line.substr(0, pos);
    if (!_config_file.eof())
      _current_line.append("\n");
    buffer += _current_line;
  }
  
  const std::vector<std::string> serverBlocks = extractServerBlocks_(buffer);

  for (size_t i = 0; i < serverBlocks.size(); ++i)
    this->_servers.push_back(parseServerBlock_(serverBlocks[i]));

  printServers();

  // Uncomment this block once the configuration file parsing has been parsed.
  /*
  if (this->_servers.empty()) {
    throwWithLog(LOG_ERROR, "No server blocks found in config file");
    }
    */
}
  
const std::vector<std::string>  Config::extractServerBlocks_(const std::string& buffer) const
{
  std::vector<std::string> blocks;
  size_t  pos = 0;
  size_t  i = 0;

  while ((pos = buffer.find("server", pos)) != std::string::npos)
  {
    // Vérifie que "server" est suivi d'une accolade
    size_t braceStart = buffer.find("{", pos);

    if (braceStart == std::string::npos)
      throwWithLog(LOG_ERROR, "Expected '{' after 'server' at position " + toString(__LINE__));
    if (pos != i || !this->blockServerIsValid_(buffer, braceStart, pos))
      throwWithLog(LOG_ERROR, "Unknown directive at position " + toString(__LINE__));

    // Trouver la fin du bloc avec gestion des accolades imbriquéesint
    int depth = 1;
    i = braceStart + 1;

    while (i < buffer.size() && depth > 0)
    {
      if (buffer[i] == '{')
        depth++;
      else if (buffer[i] == '}')
        depth--;
      ++i;
    }

    if (depth != 0)
      throwWithLog(LOG_ERROR, "Unmatched braces in server block starting at position " + toString(__LINE__));

    blocks.push_back(buffer.substr(pos, i - pos));
    // TODO addede
    while (i < buffer.size() && buffer.at(i) == '\n')
      i++;
    pos = i; // Continuer après ce blockServerIsValid
  }
  if (!buffer.substr(i).empty())
    throwWithLog(LOG_ERROR, "Unknown directive at position " + toString(__LINE__));
  return blocks;
}

bool  Config::blockServerIsValid_(const std::string& buffer, const size_t& braceStart, \
                                    const size_t& pos) const
{
  if (braceStart == pos + 6) // server{
    return (true);
  else // server { ou server a{
  {
    std::istringstream iss(buffer.substr(pos));
    std::string     arg, key;
    iss >> key >> arg;

    if (arg.at(0) == '{') // server {
      return (true);
    return (false); // server arg{
  }

  return (false);
}

ServerConfig  Config::parseServerBlock_(const std::string& serverBlock) const
{
  ServerConfig config;
  std::string  serverContent = extractServerBlockContent_(serverBlock);
  
  // extract the location bloc
  std::vector<std::string> locationBlocks = extractLocationBlocks_(serverContent);

  // remove the location blocks from root content
  for (std::vector<std::string>::const_iterator it = locationBlocks.begin(); \
      it != locationBlocks.end(); ++it)
  {
    size_t pos = serverContent.find(*it);
    if (pos != std::string::npos)
      serverContent.erase(pos, it->length());
  }

  // parse the directives for root
  parseDirectivesInServerBlock_(serverContent, config);
  
  // parse the location blocks
  for (std::vector<std::string>::iterator it = locationBlocks.begin(); it != locationBlocks.end(); ++it)
    parseDirectivesInLocationBlock_(*it, config);

  // location must always have a location with path = "/"
  if (config.locations.find("/") == config.locations.end())
    makeDefaultLocation_(config);

  return (config);
}

std::string Config::extractServerBlockContent_(const std::string& serverBlock) const
{
  size_t start = serverBlock.find('{');
  size_t end = serverBlock.rfind('}');

  if (start == std::string::npos || end == std::string::npos || end <= start)
      throwWithLog(LOG_ERROR, "Invalid server block format at position " + toString(__LINE__));

  return (serverBlock.substr(start + 1, end - start - 1));
}

std::vector<std::string>  Config::extractLocationBlocks_(const std::string& serverContent) const
{
  std::vector<std::string> locations;
  size_t pos = 0;

  while ((pos = serverContent.find("location", pos)) != std::string::npos)
  {
    // Vérifie que "location" est suivi d'une accolade
    size_t braceStart = serverContent.find("{", pos);
    if (braceStart == std::string::npos)
      throwWithLog(LOG_ERROR, "Expected '{' after 'server' at position " + toString(__LINE__));
    if (!this->LocationBlockIsValid_(serverContent, pos))
      throwWithLog(LOG_ERROR, "Unknown directive or argument invalid at position " + toString(__LINE__));

    int depth = 1;
    size_t i = braceStart + 1;
    while (i < serverContent.size() && depth > 0)
    {
      if (serverContent[i] == '{')
        depth++;
      else if (serverContent[i] == '}')
        depth--;
      ++i;
    }

    if (depth != 0)
      throwWithLog(LOG_ERROR, "Unmatched braces in location block at position " + toString(__LINE__));

    locations.push_back(serverContent.substr(pos, i - pos));
    pos = i;
  }

  return locations;
}

bool  Config::LocationBlockIsValid_(const std::string& serverContent, const size_t& pos) const
{
  std::istringstream iss(serverContent.substr(pos));
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

void  Config::parseDirectivesInServerBlock_(const std::string& serverContentWithoutLoc, ServerConfig& config) const
{
  std::istringstream iss(serverContentWithoutLoc);
  std::string        directive;

  // create a function that inilializes the server data
  initServerData_(config);

  while (std::getline(iss, directive, ';'))
  {
    directive = trim(directive);
        
    std::istringstream lineStream(directive);
    std::string key;
    lineStream >> key;

    // C'est pour gerer les cas comme : `listen 8082;;       ;`
    if (key.empty())
      continue;

    std::vector<std::string> value;
    std::string token;
    while (lineStream >> token)
      value.push_back(token);
    
    parseServerDirective_(key, value, config);
  }

  // the server must have a <host> value
  if (config.host.empty())
    throwWithLog(LOG_FATAL, "The server must have a host value at position " + toString(__LINE__));
}

void  Config::initServerData_(ServerConfig& config) const
{
  config.host = "";
  config.autoindex = false;
  config.client_max_body_size = 1048576;
  config.port = 0;
  config.root = "";
  config.server_name = "";
  config.upload_dir = "";
  config.indexs.push_back("index.html");
  config.hasValue.indexs = false;
  config.hasValue.listen = false;
  config.hasValue.redirect = false;
}

void  Config::parseServerDirective_(const std::string& key, const std::vector<std::string>& value, ServerConfig& config) const
{ 
  if (key == "server_name" || key == "root" || key == "upload_dir" || key == "autoindex" || \
      key == "client_max_body_size") // function from directives affectation
    assignValueInServer_(key, value, config);
  else if (key == "index" || key == "error_page") // function from  directives concatenation
    concatenateValueInServer_(key, value, config);
  else if (key == "listen" || key == "return")
    checkDuplicationAndAssignValueInSrv(key, value, config);
  else
    throwWithLog(LOG_ERROR, "Unknown directive '" + key + \
                              "' in server block at position " + toString(__LINE__));
}

void  Config::assignValueInServer_(const std::string& key, const std::vector<std::string>& value, ServerConfig& config) const
{
  if (key == "server_name") {
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": Invalid argument count at position " + toString(__LINE__));
    config.server_name = value[0];
  }
  else if (key == "root") {
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": Invalid argument count at position " + toString(__LINE__));
    config.root = value[0];
  }
  else if (key == "upload_dir") {
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": Invalid argument count at position " + toString(__LINE__));
    config.upload_dir = value[0];
  }
  else if (key == "autoindex") {
    if (value.size() == 0 || value.size() > 1 || (value[0] != "on" && value[0] != "off"))
      throwWithLog(LOG_ERROR, key + ": Invalid argument count at position " + toString(__LINE__));
    if (value[0] == "on")
      config.autoindex = true;
    else
      config.autoindex = false;
  }
  else if (key == "client_max_body_size") {
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": Invalid argument count at position " + toString(__LINE__));
    config.client_max_body_size = parseSize(key, value[0]); // gère les suffixes M, K
  }
}

void  Config::concatenateValueInServer_(const std::string& key, const std::vector<std::string>& value, ServerConfig& config) const
{
  if (key == "index")
  {
    if (value.size() == 0)
      throwWithLog(LOG_ERROR, key + ": Invalid argument count at position " + toString(__LINE__));
    if (config.hasValue.indexs == false) {
      config.indexs.clear();
      config.hasValue.indexs = true;
    }
    config.indexs.insert(config.indexs.end(), value.begin(), value.end());
  }
  else if (key == "error_page")
  {
    if (value.size() != 2)
      throwWithLog(LOG_ERROR, key + ": Invalid argument count at position " + toString(__LINE__));
    int code = stringToInt(key, value[0]);
    config.error_pages[code] = value[1];
  }
}

void  Config::checkDuplicationAndAssignValueInSrv(const std::string& key, const std::vector<std::string>& value, ServerConfig& config) const
{
  if (key == "listen")
  {
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": Invalid argument count at position " + toString(__LINE__));
    if (config.hasValue.listen == true)
      throwWithLog(LOG_ERROR, key + ": there is a duplication at poistion " + toString(__LINE__));

    size_t colon = value[0].find(":");
    std::string portStr;
    if (colon == std::string::npos)
    {
      if (value[0].find(".") != std::string::npos)
        throwWithLog(LOG_ERROR, key + " Invalid port format: " + value[0] + " at position " + toString(__LINE__));
      config.host = "0.0.0.0";
      portStr = value[0];
    }
    else {
      config.host = value[0].substr(0, colon);
      portStr = value[0].substr(colon + 1);
    }
    if (portStr.empty())
      throwWithLog(LOG_ERROR, key + " Invalid listen format: " + value[0] + " at position " + toString(__LINE__));
    config.port = stringToInt("port", portStr); // C++98-compatible stoi
    config.hasValue.listen = true;
  }
  else if (key == "return")
  {
    if (value.size() == 0 || value.size() > 2)
      throwWithLog(LOG_ERROR, key + ": Invalid argument count at position " + toString(__LINE__));
    if (config.hasValue.redirect == true)
      throwWithLog(LOG_ERROR, key + ": there is a duplication at poistion " + toString(__LINE__));
    int code = stringToInt(key, value[0]);
    if (value.size() == 1)
      config.redirect[code] = "";
    else
      // TODO : verifier si la valeur de value[1] = "..." ou /... ou http(s)://
      config.redirect[code] = value[1];
    config.hasValue.redirect = true;
  }
}

void  Config::parseDirectivesInLocationBlock_(std::string &locationBlock, ServerConfig &config) const
{
  LocationConfig  location_config;

  locationBlock = insertSpaceBeforeBrace_(locationBlock);
  locationBlock = extractLocationBlockContent_(locationBlock, location_config.path);
  
  std::istringstream contentStream(locationBlock);
  std::string directive;

  inheritServerDirectives_(location_config, config);
  
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


    parseLocationDirective_(key, value, location_config);
  }

  // location must always have a value in his variable path
  if (location_config.path.empty())
    throwWithLog(LOG_ERROR, "path of location is empty at position " + toString(__LINE__));
  if (config.locations.find(location_config.path) == config.locations.end())
    config.locations[location_config.path] = location_config;
  else
    throwWithLog(LOG_ERROR, "");
}

std::string Config::insertSpaceBeforeBrace_(const std::string& locationBlock) const
{
  std::string result;

  for (size_t i = 0; i < locationBlock.size(); ++i)
  {
    if (locationBlock[i] == '{')
    {
      // if the preceding character is not a space
      if (i > 0 && locationBlock[i - 1] != ' ')
        result += ' ';
    }
    result += locationBlock[i];
  }
  return result;
}

std::string Config::extractLocationBlockContent_(const std::string& locationBlock, std::string& path) const
{
  size_t start = locationBlock.find('{');
  size_t end = locationBlock.rfind('}');
  if (start == std::string::npos || end == std::string::npos || end <= start)
        throwWithLog(LOG_ERROR, "Invalid server block format at poistion " + toString(__LINE__));
  if (start != std::string::npos)
  {
    std::string loc = locationBlock.substr(0, start);
    std::istringstream  iss(loc);
    std::string key;

    iss >> key >> path;
  }
  return locationBlock.substr(start + 1, end - start - 1);
}

void  Config::inheritServerDirectives_(LocationConfig& location_config, const ServerConfig& config) const
{
  location_config.indexs = config.indexs;
  location_config.root = config.root;
  location_config.error_pages = config.error_pages;
  location_config.client_max_body_size = config.client_max_body_size;
  location_config.hasValue.path = false;
  location_config.hasValue.redirect = false;
  location_config.hasValue.indexs = false;
}

void  Config::parseLocationDirective_(const std::string& key, const std::vector<std::string>& value, LocationConfig& location_config) const
{
  if (key == "root" || key == "client_max_body_size" || key == "upload_dir" \
      || key == "cgi_extension" || key == "cgi_path" || key == "autoindex" || key == "methods")
    assignValueInLocation(key, value, location_config);
  else if (key == "index" || key == "error_page")
    concatenateValueInLocation(key, value, location_config);
  else if (key == "path" || key == "return")
    checkDuplicationAndAssignValueInLoc(key, value, location_config);
  else
    throwWithLog(LOG_ERROR, "Unknown directive '" + key + "' in location block at position" + toString(__LINE__));
}


void  Config::assignValueInLocation(const std::string& key, const std::vector<std::string>& value, LocationConfig& location_config) const
{
  if (key == "root") {
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": argument is invalid at position " + toString(__LINE__));
    location_config.root = value[0];
  }
  else if (key == "client_max_body_size") {
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": argument is invalid at position " + toString(__LINE__));
    location_config.client_max_body_size = parseSize(key, value[0]);
  }
  else if (key == "upload_dir") {
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": argument is invalid at position " + toString(__LINE__));
    location_config.upload_dir = value[0];
  }
  else if (key == "cgi_extension") {
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": argument is invalid at position " + toString(__LINE__));
    location_config.cgi_extension = value[0];
  }
  else if (key == "cgi_path") {
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": argument is invalid at position " + toString(__LINE__));
    location_config.cgi_path = value[0];
  }
  else if (key == "autoindex") {
    if (value.size() == 0 || value.size() > 1 || (value[0] != "on" && value[0] != "off"))
      throwWithLog(LOG_ERROR, key + ": argument is invalid at position " + toString(__LINE__));
    if (value[0] == "on")
      location_config.autoindex = true;
    else
      location_config.autoindex = false;
  }
  else if (key == "methods") {
    if (value.size() == 0)
      throwWithLog(LOG_ERROR, key + ": argument is invalid at position " + toString(__LINE__));
    if (!areHttpMethod(value))
      throwWithLog(LOG_ERROR, "argument is invelid at position " + toString(__LINE__));
    location_config.methods = value;
  }
}

bool  Config::areHttpMethod(const std::vector<std::string>& methods) const
{
  for (std::vector<std::string>::const_iterator it = methods.begin(); it != methods.end(); it++)
  {
    if (*it != "GET" && *it != "POST" && *it != "DELETE" && *it != "PUT"
      && *it != "HEAD" && *it != "CONNECT" && *it != "OPTIONS" && *it != "TRACE"
      && *it != "PATCH")
      return (false);
  }
  return (true);
}

void  Config::concatenateValueInLocation(const std::string& key, const std::vector<std::string>& value, LocationConfig& location_config) const
{
  if (key == "index")
  {
    if (value.size() == 0)
      throwWithLog(LOG_ERROR, key + ": argument is invalid at position " + toString(__LINE__));
    if (location_config.hasValue.indexs == false) {
      location_config.indexs.clear();
      location_config.hasValue.indexs = true;
    }
    location_config.indexs.insert(location_config.indexs.end(), value.begin(), value.end());
  }

  else if (key == "error_page")
  {
    if (value.size() != 2)
      throwWithLog(LOG_ERROR, key + ": argument is invalid at position " + toString(__LINE__));
    int code = stringToInt(key, value[0]);
    location_config.error_pages[code] = value[1];
  }
}

void  Config::checkDuplicationAndAssignValueInLoc(const std::string& key, const std::vector<std::string>& value, LocationConfig& location_config) const
{
  if (key == "path")
  {
    if (location_config.hasValue.path == true)
      throwWithLog(LOG_ERROR, key + ": there is a duplication at position " + toString(__LINE__));
    if (value.size() == 0 || value.size() > 1)
      throwWithLog(LOG_ERROR, key + ": argument is invalid at position " + toString(__LINE__));
    location_config.path = value[0];
    location_config.hasValue.path = true;
  } 
  else if (key == "return")
  {
    if (location_config.hasValue.redirect == true)
      throwWithLog(LOG_ERROR, key + ": there is a duplication at position " + toString(__LINE__));
    if (value.size() == 0 || value.size() > 2)
      throwWithLog(LOG_ERROR, key + ": argument is invalid at position " + toString(__LINE__));
    int code = stringToInt(key, value[0]);
    if (value.size() == 1)
      location_config.redirect[code] = "";
    else
      // TODO : verifier si la valeur de value[1] = "..." ou /... ou http(s)://
      location_config.redirect[code] = value[1];
    location_config.hasValue.redirect = true;
  }
}

void  Config::makeDefaultLocation_(ServerConfig& config) const
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



  // you can use this function to add manually a serverconfig without parsing
void  Config::createServerConfigManually(void)
{
  logger(LOG_INFO, "Creating server config manually");
  ServerConfig  result;
  result.server_name = "localhost";
  result.client_max_body_size = 10485760;
  result.error_pages[404] = "/404.html";
  result.host = "127.0.0.1";
  result.port = 8080; result.root = "./srandria";

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

// TODO : move trim in utils.cpp
std::string trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\r\n");
    size_t last = str.find_last_not_of(" \t\r\n");
    if (first == std::string::npos || last == std::string::npos)
        return "";
    return str.substr(first, last - first + 1);
}


// TODO : move stringToInt in utils.cpp
int stringToInt(const std::string& key, const std::string& str)
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

// TODO : move parseSize in utils.cpp
size_t  parseSize(const std::string& key, const std::string& str)
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
    for (size_t i = 0; i < this->_servers.size(); ++i)
    {
        std::cout << std::endl << std::endl;
        const ServerConfig& config = this->_servers[i];

        std::cout << "server[" << i << "].host = |" << config.host << "|" << std::endl;
        std::cout << "server[" << i << "].port = |" << config.port << "|" << std::endl;
        std::cout << "server[" << i << "].root = |" << config.root << "|" << std::endl;

        std::cout << "server[" << i << "].index = |";
        for (size_t j = 0; j < config.indexs.size(); ++j)
        {
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

        for (std::map<int, std::string>::const_iterator ep = config.error_pages.begin(); ep != config.error_pages.end(); ++ep)
            std::cout << "server[" << i << "].error_pages[" << ep->first << "] = |" << ep->second << "|" << std::endl;

        for (std::map<std::string, LocationConfig>::const_iterator loc = config.locations.begin(); loc != config.locations.end(); ++loc)
        {
            const LocationConfig& lcfg = loc->second;
            std::string path = loc->first;

            std::cout << "server[" << i << "].locations[" << path << "].path = |" << lcfg.path << "|" << std::endl;
            std::cout << "server[" << i << "].locations[" << path << "].root = |" << lcfg.root << "|" << std::endl;

            std::cout << "server[" << i << "].locations[" << path << "].index = |";
            for (size_t j = 0; j < lcfg.indexs.size(); ++j)
            {
                std::cout << lcfg.indexs[j];
                if (j + 1 < lcfg.indexs.size()) std::cout << ", ";
            }
            std::cout << "|" << std::endl;

            for (std::map<int, std::string>::const_iterator ep = lcfg.redirect.begin(); ep != lcfg.redirect.end(); ++ep)
            {
                std::cout << "server[" << i << "].locations[" << path << "].redirect[" << ep->first << "] = |" << ep->second << "|" << std::endl;
            }

            std::cout << "server[" << i << "].locations[" << path << "].methods = |";
            for (size_t j = 0; j < lcfg.methods.size(); ++j)
            {
                std::cout << lcfg.methods[j];
                if (j + 1 < lcfg.methods.size()) std::cout << ", ";
            }
            std::cout << "|" << std::endl;

            std::cout << "server[" << i << "].locations[" << path << "].autoindex = |" << (loc->second.autoindex ? "true" : "false") << "|" << std::endl;
            std::cout << "server[" << i << "].locations[" << path << "].upload_dir = |" << loc->second.upload_dir << "|" << std::endl;
            std::cout << "server[" << i << "].locations[" << path << "].client_max_body_size = |" << loc->second.client_max_body_size << "|" << std::endl;
            std::cout << "server[" << i << "].locations[" << path << "].cgi_extension = |" << loc->second.cgi_extension << "|" << std::endl;
            std::cout << "server[" << i << "].locations[" << path << "].cgi_path = |" << loc->second.cgi_path << "|" << std::endl;

            for (std::map<int, std::string>::const_iterator ep = lcfg.error_pages.begin(); ep != lcfg.error_pages.end(); ++ep)
            {
                std::cout << "server[" << i << "].locations[" << path << "].error_pages[" << ep->first << "] = |" << ep->second << "|" << std::endl;
            }
        }

        std::cout << std::endl << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    }
}
