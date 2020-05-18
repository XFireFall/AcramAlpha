#include <iostream>
#include <sstream>
#include "scriptparser.hpp"


//----------------------------------------------------------------

bool commutative(std::string const& op)
{
#define COMM(name) \
	if (op == #name) return true;
#include "scripts/relation.dat"
	return false;
}

//--------------------------------

bool associative(std::string const& op)
{
#define SASS(name) \
	if (op == #name) return true;
#include "scripts/relation.dat"
	return false;
}

//----------------------------------------------------------------

int priority(std::string const& op, int argc)
{
	if (argc == 1) {
#define FUNC1(name, cname, prio, diff, tex) \
		if (op == #name) return (prio);
#include "scripts/functions.dat"
	} else {
#define FUNC2(name, cname, prio, diff, tex) \
		if (op == #name) return (prio);
#include "scripts/functions.dat"
	}
	return 0;
}

//----------------------------------------------------------------

bool maybeOP(std::string const& s)
{
	int len = s.size();
#define FUNC1(name, cname, prio, diff, tex) if (s == std::string(#name).substr(0, len)) return true;
#define FUNC2(name, cname, prio, diff, tex) if (s == std::string(#name).substr(0, len)) return true;
#include "scripts/functions.dat"
	return false;
}

//--------------------------------

bool isdefOP(std::string const& s)
{
#define FUNC1(name, cname, prio, diff, tex) if (s == #name) return true;
#define FUNC2(name, cname, prio, diff, tex) if (s == #name) return true;
#include "scripts/functions.dat"
	return false;
}

//--------------------------------

std::string adv_getOP(std::ifstream& in)
{
	std::string res = "";
	
	while (std::isspace(in.peek()))
		in.get();
	
	while (maybeOP(res))
		res.push_back(in.get());
	in.putback(res.back());
	res.pop_back();
	
	if (isdefOP(res))
		return res;
	
	while (res != "") {
		in.putback(res.back());
		res.pop_back();
	}
	
	return "";
}

//----------------------------------------------------------------

Equation getARG(std::ifstream& in)
{
	char c;
	while(std::isspace(c = in.get()))
		;
	
	if (std::isdigit(c)) {
		in.unget();
		double d;
		in >> d;
		return d;
		
	} else if (c == 'x') {
		return 'x';
		
	} else if (c == '(') {
		Equation res = getEQ(in);
		if ((c = in.get()) != ')')
			throw std::runtime_error("Syntax error: ')' expected, got " + std::string{1, c});
		return Equation{"br", res};
		
	} else {
		in.unget();
		
		std::string op = adv_getOP(in);
		if (op == "") {
			std::stringstream ss;
			ss << in.rdbuf();
			throw std::runtime_error("Syntax error: parsing stopped, remains: \"" + ss.str() + "\"");
		}
		
		return Equation{op, getARG(in)};
	}
	
	return Equation{};
}

//--------------------------------

Equation getEQ(std::ifstream& in)
{
	std::string op;
	Equation arg;
	
	Equation res = getARG(in);
	Equation *parg = res.deepright();
// 	std::cout << res << " parg=" << *parg << std::endl;
	while (!in.eof()) {
		op = adv_getOP(in);
		
		if (in.eof())
			break;
		
		if (op != "") {
			arg = getARG(in);
		} else if (in.peek() == 'x') {
			in.get();
			op = "*";
			arg = 'x';
		} else
			break;
		
		if (::priority(op, 2) <= parg->relativepriority(Equation::RIGHT)) {
// 			std::cout << op << " <= " << *parg << std::endl;
			Equation **pp = parg->unary() ? &parg->_left : &parg->_right;
			Equation* tmp = new Equation{**pp, op, arg};
			delete *pp;
			*pp = tmp;
		} else {
// 			std::cout << op << " >  " << *parg << std::endl;
			res = Equation{res, op, arg};
		}
		
		parg = res.deepright();
// 		std::cout << std::endl << res << " parg=" << *parg << std::endl;
	}
	return res;
}

//----------------------------------------------------------------
