/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zramahaz <zramahaz@student.42antanana>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 09:06:02 by srandria          #+#    #+#             */
/*   Updated: 2025/09/12 16:17:25 by zramahaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "../utils/utils.hpp"
#include <cstddef>
#include <string>

struct DirectiveStatusLocation { // this is to manage duplicates
    bool  path;
    bool  redirect;
    bool  indexs;
};

struct LocationConfig {
    std::string                 path;                 // Chemin de la location (ex: "/", "/upload")
    std::string                 root;                 // Racine des fichiers pour cette location
    bool                        autoindex;            // Si true, liste les répertoires (comme "ls")
    std::vector<std::string>    indexs;               // Fichier par défaut (ex: "index.html")
    std::string                 upload_dir;           // Dossier pour les uploads (POST)
    std::map<int, std::string>  redirect;             // URL de redirection (ex: "301 http://example.com")
    std::string                 cgi_extension;        // Extension pour CGI (ex: ".py")
    std::string                 cgi_path;             // Chemin de l'interpréteur (ex: "/usr/bin/python3")
    std::vector<std::string>    methods;              // Méthodes autorisées (GET, POST, DELETE)
    std::map<int, std::string>  error_pages;
    size_t                      client_max_body_size; // Taille max du body (ex: 1048576 pour 1MB)
    DirectiveStatusLocation     hasValue;

    LocationConfig() : autoindex(false) {}
};

struct DirectiveStatusServer { // this is to manage duplicates
    bool  listen;
    bool  redirect;
    bool  indexs;
};

struct ServerConfig {
    std::string                           host;                 // Adresse d'écoute (ex: "127.0.0.1")
    int                                   port;                 // Port d'écoute (ex: 8080)
    std::string                           server_name;          // Nom du serveur (ex: "localhost")
    bool                                  autoindex;            // Si true, liste les répertoires (comme "ls")
    std::string                           upload_dir;           // Dossier pour les uploads (POST)
    std::map<int, std::string>            redirect;             // URL de redirection (ex: "301 http://example.com")
    std::string                           root;                 // Racine par défaut des fichiers
    std::vector<std::string>              indexs;               // Fichier index par défaut    
    size_t                                client_max_body_size; // Taille max du body (ex: 1048576 pour 1MB) 
    std::map<int, std::string>            error_pages;          // Pages d'erreur (ex: 404 -> "/404.html")
    DirectiveStatusServer                       hasValue;
    std::map<std::string, LocationConfig> locations;            // Configs par location
};

class Config
{
  private:
    Config(void);
    Config(const Config& other);
    Config& operator=(const Config& other);

    void  load_(void);                              // Charge et parse le fichier de configuration
    bool  isValid_(void) const;                     // Vérifie si la configuration est valide
    void  skipWhiteSpace_(void);
    // you can use this function to add manually a serverconfig without parsing
    void  createServerConfigManually(void);

    // zramahaz function
    const std::vector<std::string>  extractServerBlocks_(const std::string& buffer) const;
    bool                      blockServerIsValid_(const std::string& buffer, const size_t& braceStart, const size_t& pos) const;
    ServerConfig              parseServerBlock_(const std::string& serverBlock) const;
    std::string               extractServerBlockContent_(const std::string& serverBlock) const;
    std::vector<std::string>  extractLocationBlocks_(const std::string& serverContent) const;
    bool                      LocationBlockIsValid_(const std::string& serverContent, const size_t& pos) const;
    void                      parseDirectivesInServerBlock_(const std::string& serverContentWithoutLoc, ServerConfig& config) const;
    void                      initServerData_(ServerConfig& config) const;
    void                      parseServerDirective_(const std::string& key, const std::vector<std::string>& value, ServerConfig& config) const;
    void                      assignValueInServer_(const std::string& key, const std::vector<std::string>& value, ServerConfig& config) const;
    void                      concatenateValueInServer_(const std::string& key, const std::vector<std::string>& value, ServerConfig& config) const;
    void                      checkDuplicationAndAssignValueInSrv(const std::string& key, const std::vector<std::string>& value, ServerConfig& config) const;
    void                      parseDirectivesInLocationBlock_(std::string &locationBlock, ServerConfig &config) const;
    std::string               insertSpaceBeforeBrace_(const std::string& locationBlock) const;
    std::string               extractLocationBlockContent_(const std::string& locationBlock, std::string& path) const;
    void                      inheritServerDirectives_(LocationConfig& location_config, const ServerConfig& config) const;
    void                      parseLocationDirective_(const std::string& key, const std::vector<std::string>& value, LocationConfig& location_config) const;
    void                      assignValueInLocation(const std::string& key, const std::vector<std::string>& value, LocationConfig& location_config) const;
    void                      concatenateValueInLocation(const std::string& key, const std::vector<std::string>& value, LocationConfig& location_config) const;
    void                      checkDuplicationAndAssignValueInLoc(const std::string& key, const std::vector<std::string>& value, LocationConfig& location_config) const;
    void                      makeDefaultLocation_(ServerConfig& config) const;

    void                      printServers(void) const;
    
    std::vector<ServerConfig> _servers;     // Tous les serveurs configurés
    const std::string         _config_path; // chemin pour le fichier de configuration
    std::ifstream             _config_file; // fd pour le fichier
    std::string               _current_line; // ligne actuelle dans le fichier
    size_t                    _line_number; // nombre de ligne dans le fichier
    
    
    public:
    Config(std::string argv1);
    ~Config(void);
    
    // Getter pour accéder à la configuration parsée
    const std::vector<ServerConfig>& getServers() const;
  };

  std::string               trim(const std::string& str);
  int                       stringToInt(const std::string& key, const std::string& str);
  size_t                    parseSize(const std::string& key, const std::string& str);

#endif
