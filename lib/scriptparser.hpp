#pragma once
#include <fstream>
#include <string>
#include "Equation.hpp"


/* checks if (a+b)+c==a+(b+c) according to scripts */
bool commutative(std::string const& op);

/* checks if a+b==b+a according to scripts */
bool associative(std::string const& op);

/* gets operations priority according to scripts */
int priority(std::string const& op, int argc);

/* parses an equation from a file stream */
Equation getEQ(std::ifstream& in);
