#ifndef FLAGSH
#define FLAGSH
#include "filework.h"

bool isFlag(const std::string&);
std::vector<std::string> getParameters(std::vector<std::string>&,
	const std::vector<std::string>& );
void getNameAfterFlag(const std::vector<std::string>&,
	const std::string&,std::string&);
void flag_func_list();
#endif