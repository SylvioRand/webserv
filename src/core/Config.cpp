/* ************************************************************************** */ /*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 09:06:23 by srandria          #+#    #+#             */
/*   Updated: 2025/07/16 10:04:50 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
  
#include "../../include/core/Config.hpp"

void Config::skipWhiteSpace() {
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

void Config::load(const std::string& filepath) {
    _config_file_path = filepath;
    _config_file.open(filepath.c_str());
    if (!_config_file.is_open()) {
        throw std::runtime_error("Failed to open config file: " + filepath);
    }

    while (std::getline(_config_file, _current_line)) {
        _line_number++;
        this->skipWhiteSpace();
        if (_current_line.empty() || _current_line[0] == '#') {
            continue;
        }

        if (_current_line.find("server") == 0) {
            this->parseServerBlock();
        }
    }

    if (_servers.empty()) {
        throw std::runtime_error("No server blocks found in config file");
    }
}


const std::vector<ServerConfig>& Config::getServers() const
{
  return (_servers);
}

bool Config::isValid() const {
    for (size_t i = 0; i < _servers.size(); ++i) {
        const ServerConfig& s = _servers[i];
        
        // Vérifie que chaque serveur a au moins un port d'écoute
        if (s.port < 1 || s.port > 65535)
            return false;
        
        // Vérifie les tailles maximales de body
        if (s.client_max_body_size > MAX_BODY_LIMIT)
            return false;
        
        // Vérifie qu`on a location /`
        if (s.locations.find("/") == s.locations.end())
            return false;
    }
    return true;
}


// Zramahaz’s implementation starts here.

// This function serves as the entry point for the configuration file parser.
void parseServerBlock()
{}
