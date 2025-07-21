/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: srandria <srandria@student.42antananarivo  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 09:06:02 by srandria          #+#    #+#             */
/*   Updated: 2025/07/21 10:07:50 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#define MAX_BODY_LIMIT (100 * 1024 * 1024) 

#include "../utils/utils.hpp"

struct LocationConfig {
    std::string               path;           // Chemin de la location (ex: "/", "/upload")
    std::string               root;           // Racine des fichiers pour cette location
    bool                      autoindex;      // Si true, liste les répertoires (comme "ls")
    std::string               index;          // Fichier par défaut (ex: "index.html")
    std::string               upload_dir;     // Dossier pour les uploads (POST)
    std::string               redirect;       // URL de redirection (ex: "301 http://example.com")
    std::string               cgi_extension;  // Extension pour CGI (ex: ".py")
    std::string               cgi_path;       // Chemin de l'interpréteur (ex: "/usr/bin/python3")
    std::vector<std::string>  methods;        // Méthodes autorisées (GET, POST, DELETE)

    LocationConfig() : autoindex(false) {}
};

struct ServerConfig {
    std::string                           host;                 // Adresse d'écoute (ex: "127.0.0.1")
    int                                   port;                 // Port d'écoute (ex: 8080)
    std::string                           server_name;          // Nom du serveur (ex: "localhost")
    std::string                           root;                 // Racine par défaut des fichiers
    std::string                           index;                // Fichier index par défaut
    size_t                                client_max_body_size; // Taille max du body (ex: 1048576 pour 1MB)
    std::map<int, std::string>            error_pages;          // Pages d'erreur (ex: 404 -> "/404.html")
    std::map<std::string, LocationConfig> locations;            // Configs par location

    ServerConfig();
};

class Config
{
  private:
    Config(void);
    Config(const Config& other);
    Config& operator=(const Config& other);

    void load_(void);                       // Charge et parse le fichier de configuration
    bool isValid_(void) const;              // Vérifie si la configuration est valide
    void parseServerBlock_(void);           // parse de zramahaz
    void skipWhiteSpace_(void);

    std::vector<ServerConfig> _servers;     // Tous les serveurs configurés
    const std::string         _config_path;
    std::ifstream             _config_file;
    std::string               _current_line;
    size_t                    _line_number;


  public:
    Config(std::string argv1);
    ~Config(void);

    // Getter pour accéder à la configuration parsée
    const std::vector<ServerConfig>& getServers() const;

};

#endif
