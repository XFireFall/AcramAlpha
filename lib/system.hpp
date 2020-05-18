#pragma once
#include <string>

#if defined(__linux) || defined(__linux__)
/* execute a Linux command ang get its console output */
std::string exec(std::string const& cmd);

/* overdrive for ::system(char const*) */
int system(std::string const& cmd);

/* true if the specified package is installed on the computer */
bool isinstalled(std::string const& pkg);

/* returns full path to a command - or "" if there's none */
std::string whereis(std::string const& srh, std::string const& pkg);

/* add an extansion or change the one if there is any */
std::string extension(std::string const& fname, std::string const& ext);
#endif
