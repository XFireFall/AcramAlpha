#if defined(__linux) || defined(__linux__)


#include "system.hpp"

#include <cstdio>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>


//--------------------------------

std::string exec(std::string const& cmd)
{
	std::string res;
	char buff[128] = "";
	
	std::unique_ptr<FILE, decltype(&pclose)> pipe{popen(cmd.c_str(), "r"), pclose};
	if (!pipe)
		throw std::runtime_error("exec: popen() failed!");
	
	while (fgets(buff, sizeof(buff), pipe.get()) != nullptr)
		res += buff;
	return res;
}

//--------------------------------

int system(std::string const& cmd)
{
	return system(cmd.c_str());
}

//--------------------------------

bool isinstalled(std::string const& pkg)
{
	bool installed = !system("dpkg -s " + pkg + " >/dev/null 2>&1");
	if (!installed) {
		std::cerr << "external error: '" + pkg + "' not found" << std::endl;
		std::clog << "Please install package'" + pkg + "' and retry" << std::endl;
	}
	return installed;
}

//--------------------------------

std::string whereis(std::string const& srh, std::string const& pkg)
{
	if (!isinstalled(pkg))
		return "";
	
	std::string res;
	std::stringstream ss{exec("whereis -b " + srh)};
	ss >> res >> res;
	
	if (res == srh + ":") {
		std::cerr << "unexpected error: '" + srh + "' not found though package '" << pkg << "' is installed" << std::endl;
		res = "";
	}
	
	return res;
}

//--------------------------------

std::string extension(std::string const& fname, std::string const& ext)
{
	int const pos = fname.find_last_of('.');
	return (pos ? fname.substr(0, pos) : fname) + "." + ext;
}

//--------------------------------

#endif
