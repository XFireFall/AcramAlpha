#pragma once
#include <fstream>

class Texfile: public std::ofstream
{
private:
	std::string name;
	
public:
	Texfile(std::string const& filename);
	~Texfile(void);
	
	void close(void);
	
	/* closes the file and makes an attempt to convert .tex file to .pdf */
	int move2pdf(void);
};
