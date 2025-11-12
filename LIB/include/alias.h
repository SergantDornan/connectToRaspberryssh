#ifndef ALIAS
#define ALIAS

#include "filework.h"
#include "algs.h"
const std::vector<std::string> configFiles = {getHomedir() + "/.bashrc", 
	getHomedir() + "/.zshrc", getHomedir() + "/.config/fish/config.fish"};
const std::string removeAliasCommand = "remove_alias";
const std::string addAliasCommand = "add_alias";
std::vector<std::string> findConfig();
std::string getLine(const std::string&, const std::string&, const std::string&);
void addAlias(const std::string&, const std::string&);
void removeAlias(const std::string&, const std::string&);
void aliasHelp(const std::string&);
int addAlias_name(const std::string& name, const std::string& arg = "");
int removeAlias_name(const std::string& name, const std::string& arg = "");
void alias_func_list();
int manage_alias(const std::string&,const std::vector<std::string>&);
std::vector<std::string> getPathByAlias(const std::string&);
std::string parseAlias(const std::string& config, const std::string& alias);
#endif