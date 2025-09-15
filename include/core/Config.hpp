/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zramahaz <zramahaz@student.42antanana>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 09:06:02 by srandria          #+#    #+#             */
/*   Updated: 2025/09/14 15:24:08 by srandria         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "../utils/utils.hpp"
#include <sys/types.h>

struct DirectiveStatusLocation
{
    bool  path;
    bool  redirect;
    bool  indexs;
};

struct LocationConfig
{
    std::string                 path;
    std::string                 root;
    bool                        autoindex;
    std::vector<std::string>    indexs;
    std::string                 upload_dir;
    std::map<int, std::string>  redirect;
    std::string                 cgi_extension;
    std::string                 cgi_path;
    std::vector<std::string>    methods;
    std::map<int, std::string>  error_pages;
    size_t                      client_max_body_size;
    DirectiveStatusLocation     hasValue;

    LocationConfig() : autoindex(false) {}
};

struct DirectiveStatusServer
{
    bool  listen;
    bool  redirect;
    bool  indexs;
};

struct ServerConfig
{
    std::string                           host;
    ssize_t                               port;
    std::string                           server_name;
    bool                                  autoindex;
    std::string                           upload_dir;
    std::map<int, std::string>            redirect;
    std::string                           root;
    std::vector<std::string>              indexs;
    size_t                                client_max_body_size;
    std::map<int, std::string>            error_pages;
    DirectiveStatusServer                 hasValue;
    std::map<std::string, LocationConfig> locations;
};

class Config
{
  private:
    Config(void);
    Config(const Config& other);
    Config& operator=(const Config& other);

    void  load_(void);
    void  checkIfValid_(void) const;
    void  skipWhiteSpace_(void);
    void  createServerConfigManually(void);

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
    bool                      areHttpMethod(const std::vector<std::string>& methods) const;
    
    std::vector<ServerConfig> _servers;
    const std::string         _config_path;
    std::ifstream             _config_file;
    std::string               _current_line;
    size_t                    _line_number;

  public:
    Config(std::string argv1);
    ~Config(void);
    
    const std::vector<ServerConfig>& getServers() const;
  };

  std::string               trim(const std::string& str);
  int                       stringToInt(const std::string& key, const std::string& str);
  size_t                    parseSize(const std::string& key, const std::string& str);

#endif
