#include <iostream>
#include <sstream>
#include "lib/Equation.hpp"
#include "lib/Texfile.hpp"
#include "lib/system.hpp"


int main(int argc, char *argv[])
{
	std::string const outall = "output.tex";
	Texfile all{outall};
	
	for (int i = 1; i < argc; ++i) {
		try{
			std::string const infile{argv[i]};
			std::cout << "Processing " << infile << "..." << std::endl;
			
			Equation eq{infile};
			Equation deq = eq.diff();
			
			std::stringstream ss;
			ss << "$$f(x)=" << eq.tex() << "$$" << std::endl << std::endl;
			ss << "$$f'(x)=" << deq.tex() << "$$" << std::endl << std::endl;
			
			Texfile out{extension(infile, "tex")};
			out << ss.str();
			out.move2pdf();

			all << "\\begin{center}\\Large File \"\\textbf{" << infile << "}\":\\end{center}" << std::endl << std::endl;
			all << ss.str() << std::endl << std::endl;
			
		} catch(std::exception& ex) {
			std::cerr << ex.what() << std::endl;
		}
	}
	
	std::cout << "Writing to " << outall << "..." << std::endl;
	all.move2pdf();
	
	return 0;
}
