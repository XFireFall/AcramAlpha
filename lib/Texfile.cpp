#include <iostream>
#include "Texfile.hpp"
#include "system.hpp"


//--------------------------------

Texfile::Texfile(std::string const& filename)
	: std::ofstream(filename)
	, name(filename)
{
	*this << "\\documentclass[14pt,a4paper]{article}" << std::endl;
	
	*this << "\\usepackage[left=1cm,right=1cm,top=2cm,bottom=2cm]{geometry}" << std::endl <<
			"\\usepackage[T1,T2A]{fontenc}" << std::endl <<
			"\\usepackage[utf8]{inputenc}" << std::endl <<
			"\\usepackage[russian, english]{babel}" << std::endl << 
			"\\usepackage{mathtext}" << std::endl <<
			"\\usepackage{amsmath,amsthm,amssymb}" << std::endl;
	
	*this << "\\begin{document}" << std::endl << std::endl;
	
	*this << "\\begin{center}\\LargeУтрем нос Стивену Вольфраму!\\end{center}" << std::endl;
}

//--------------------------------

Texfile::~Texfile(void)
{
	close();
}

//--------------------------------

void Texfile::close(void)
{
	if (!is_open())
		return;
	*this << "\\end{document}" << std::endl;
	std::ofstream::close();
}

//--------------------------------

int Texfile::move2pdf(void)
{
	close();
	
	static bool alreadyfailed = false;
	if (alreadyfailed)
		return 1;
	
#if defined(__linux) || defined(__linux__)
	if (!isinstalled("texlive-lang-cyrillic"))
		return alreadyfailed = true;
	
	alreadyfailed = system(whereis("texi2pdf", "texlive") + " " + name + " -b -q --build=clean -o " + extension(name, "pdf"));
	
	if (!alreadyfailed)
		system("rm " + name);
#else
	std::clog << "Conversion (.tex -> .pdf) failed: the function is supported only on Linux" << std::endl;
	alreadyfailed = true;
#endif
	
	return alreadyfailed;
}
