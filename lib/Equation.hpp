#pragma once
#include <ostream>
#include <string>
#include "../options.hpp"


class Equation {
private:
	double _num = 0;
	std::string _op = ""; // one of which were specified by scripts
	Equation *_left  = nullptr;
	Equation *_right = nullptr;
	
	/* NONE  - used by unary operations to be the "second" argument
	 * X     - a variable (always x)
	 * CONST - a numeric constant (_num active)
	 * OP    - an operation (_op, _left, _right active)*/
	enum types { NONE, X, CONST, OP } _type = NONE;
	
	/* LEFT  - left branch
	 * RIGHT - right branch */
	enum branch { LEFT, RIGHT };
	
private: // unstable & dangerous internals
	/* moves all data from the specified branch */
	void movefrom(branch b);
	
	/* returns pointer to the rightest operation - used while parsing */
	Equation* deepright(void);
	
	/* if var[0..N-1]==cases[0..N-1] then simplifies current equation to cases[N] */
	template<int N> bool analyze(Equation* cases[][N + 1], int size, Equation* var[N]);
	
public: // types
	enum class ex { NEFILE, UKTYPE, UKOP, SYNTAX };
	
public: // ctors & dtors & =
	Equation(void);
	Equation(char c);
	Equation(int num);
	Equation(double num);
	
	Equation(Equation const& eq);
	Equation(Equation && eq);
	
	Equation& operator=(Equation const& eq);
	Equation& operator=(Equation && eq);
	
	Equation(Equation const& a, std::string const& op, Equation const& b);
	Equation(std::string const& op, Equation const& a);
	
	~Equation(void);
	
	explicit Equation(std::string const& filename);
	
public: // const public methods
    bool operator==(Equation const& eq) const;
	bool operator!=(Equation const& eq) const;
	
	bool operator==(int num) const;
	bool operator==(double num) const;
	
	bool operator==(types t) const;
	bool operator!=(types t) const;
	
	bool operator==(std::string const& op) const;
	bool operator!=(std::string const& op) const;
	
	bool unary(void) const;		// check if the operation takes one operand only
	int priority(void) const;	// get priority according to scripts
	int relativepriority(branch b) const;	// this priprity is different from another one: a-(b+c) != a-b+c
	
public: // scripted methods
#define FUNC1(name, cname, prio, diff, tex) \
	Equation cname(void) const;
#define FUNC2(name, cname, prio, diff, tex) \
	Equation cname(Equation const& eq) const;
#include "scripts/functions.dat"
	
public: // main purpose
	Equation diff(void) const;		// produce differentiated result - automatically simplified
	void simplify(int prio = 0);	// recursively simplify the equation
	std::string tex(int prio = 100) const;	// generate LaTex formula
	
public: // excessive access functions
	friend Equation getEQ(std::ifstream& in);
	friend std::ostream& operator<<(std::ostream& os, Equation const& it);
};
