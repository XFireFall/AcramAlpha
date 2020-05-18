#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include "Equation.hpp"
#include "scriptparser.hpp"


//================================| unstable & dangerous internals |================================

void Equation::movefrom(branch b)
{
	Equation *eq = (b == LEFT) ? _left : _right;
	Equation *ll = _left, *rr = _right;
	_type = eq->_type;
	switch (_type) {
		case NONE:
		case X:
			break;
			
		case CONST:
			_num = eq->_num;
			break;
			
		case OP:
			_op = eq->_op;
			_left = eq->_left;
			_right = eq->_right;
			break;
			
		default:
			break;
	}
	
	ll->_type = CONST;
	rr->_type = CONST;
	delete ll;
	delete rr;
}

Equation* Equation::deepright(void)
{
	if (_type != OP)
		return this;
	
	if (unary()) {
		if (_op == "-" && *_left == "-" && _left->unary())
			return _left->deepright();
		
		return this;
	}
	
	if (*_right != OP)
		return this;
	
	if (*_right == OP && _right->unary() && *_right != "-")
		return this;
	
	return _right->deepright();
}

template<int N>
bool Equation::analyze(Equation* cases[][N + 1], int size, Equation* var[N])
{
	for (int k = 0; k < size; ++k) {
		bool match = true;
		for (int i = 0; i < N && match; ++i)
			match = (*var[i] == *cases[k][i]);
		if (!match)
			continue;
		
		operator=(Equation{*cases[k][N]});
		return true;
	}
	return false;
}


//================================| ctors & dtors & = |================================

Equation::Equation(void)
	: _type(NONE)
{ }

//--------------------------------

Equation::Equation(char c)
	: _type(X)
{ }

//--------------------------------

Equation::Equation(int num)
	: _type(CONST)
	, _num(num)
{ }

//--------------------------------

Equation::Equation(double num)
	: _type(CONST)
	, _num(num)
{ }

//----------------------------------------------------------------

Equation::Equation(Equation const& eq)
{
	operator=(eq);
}

//--------------------------------

Equation::Equation(Equation && eq)
{
	std::swap(_num,   eq._num);
	std::swap(_op,    eq._op);
	std::swap(_left,  eq._left);
	std::swap(_right, eq._right);
	std::swap(_type,  eq._type);
}

//----------------------------------------------------------------

Equation& Equation::operator=(Equation const& eq)
{
	if (_type == OP) {
		delete _left;
		delete _right;
	}
	
	_type = eq._type;
	switch (_type) {
		case NONE:
		case X:
			break;
			
		case CONST:
			_num = eq._num;
			break;
			
		case OP:
			_op = eq._op;
			_left  = new Equation{*eq._left};
			_right = new Equation{*eq._right};
			break;
			
		default:
			throw ex::UKTYPE;
	}
	return *this;
}

//--------------------------------

Equation& Equation::operator=(Equation && eq)
{
	std::swap(_num,   eq._num);
	std::swap(_op,    eq._op);
	std::swap(_left,  eq._left);
	std::swap(_right, eq._right);
	std::swap(_type,  eq._type);
	return *this;
}

//----------------------------------------------------------------

Equation::Equation(Equation const& a, std::string const& op, Equation const& b)
	: _type(OP)
	, _op(op)
	, _left(new Equation{a})
	, _right(new Equation{b})
{ }

//--------------------------------

Equation::Equation(std::string const& op, Equation const& a)
	: Equation(a, op, Equation{})
{ }

//----------------------------------------------------------------

Equation::~Equation(void)
{
	if (_type == OP) {
		delete _left;
		delete _right;
	}
}

//----------------------------------------------------------------

Equation::Equation(std::string const& filename)
{
	std::ifstream in{filename};
	if (!in)
		throw std::runtime_error("Could not open file \"" + filename + "\"");
	try{
		operator=(getEQ(in));
	} catch(std::exception &ex) {
		in.close();
		throw std::runtime_error(filename + ": " + ex.what());
	}
	in.close();
}


//================================| const public methods |================================

bool Equation::operator==(Equation const& eq) const
{
	if (_type != eq._type)
		return false;
	
	switch (_type) {
		case Equation::NONE:
		case Equation::X:
			return true;
			
		case Equation::CONST:
			return _num == eq._num;
			
		case Equation::OP:
			return (_op == eq._op)
					&& ((*_left == *eq._left) && (*_right == *eq._right)
						|| (commutative(_op) && (*_left == *eq._right) && (*_right == *eq._left)));
			
		default:
			throw ex::UKTYPE;
	}
	return false;
}

//--------------------------------

bool Equation::operator!=(Equation const& eq) const
{
	return !operator==(eq);
}

//----------------------------------------------------------------

bool Equation::operator==(int num) const
{
	return (_type != CONST) ? false : (_num == num);
}

//--------------------------------

bool Equation::operator==(double num) const
{
	return (_type != CONST) ? false : (_num == num);
}

//----------------------------------------------------------------

bool Equation::operator==(types t) const
{
	return _type == t;
}

//--------------------------------

bool Equation::operator!=(types t) const
{
	return _type != t;
}

//----------------------------------------------------------------

bool Equation::operator==(std::string const& op) const
{
	return _type == OP && _op == op;
}

//--------------------------------

bool Equation::operator!=(std::string const& op) const
{
	return _type != OP || _op != op;
}

//----------------------------------------------------------------

bool Equation::unary(void) const
{
	return (_type == OP) ? (_right->_type == NONE) : false;
}

//--------------------------------

int Equation::priority(void) const
{
	if (_type == OP)
		return ::priority(_op, unary() ? 1 : 2);
	
	return 0;
}

//--------------------------------

int Equation::relativepriority(branch b) const
{
	if (b == LEFT) {
		if (_op == "^")
			return priority() - 1;
		
		return priority();
	}
	
	// ~LASS(-, +)
	if (_op == "-" && !unary())
		return priority() - 1;
	
	return priority();
}


//================================| scripted methods |================================

#define FUNC1(name, cname, prio, diff, tex) \
Equation Equation::cname(void) const { return Equation{#name, *this}; }
#define FUNC2(name, cname, prio, diff, tex) \
Equation Equation::cname(Equation const& eq) const { return Equation{*this, #name, eq}; }
#include "scripts/functions.dat"


//================================| main purpose |================================

Equation Equation::diff(void) const
{
	switch (_type) {
		case NONE : return NONE;
		case X    : return 1;
		case CONST: return 0;
			
		case OP:
			{
				Equation res;
				Equation &x = *_left, &y = *_right;
				
				if (unary()) {
#define FUNC1(name, cname, prio, diff, tex) \
					if (_op == #name) res = (diff); else
#include "scripts/functions.dat"
						throw ex::UKOP;
				} else {
#define FUNC2(name, cname, prio, diff, tex) \
					if (_op == #name) res = (diff); else
#include "scripts/functions.dat"
						throw ex::UKOP;
				}
				
				res.simplify();
				return res;
			}
			
		default:
			throw std::runtime_error("Unexpected error: Equation::_type=" + (int)_type);
	}
	return NONE;
}

//--------------------------------

void Equation::simplify(int prio)
{
	switch (_type) {
		case Equation::NONE:
		case Equation::X:
			break;
			
		case Equation::CONST:
			if (_num < 0) {
				operator=(Equation{"-", -_num});
				return;
			}
			break;
			
		case Equation::OP:
			_left->simplify(priority());
			_right->simplify(priority());
			
			if (_op == "br") {
				if (*_left != OP)
					return movefrom(LEFT);
				
				if (*_left == OP && prio >= _left->priority())
					return movefrom(LEFT);
			}
			
			{ // simplify: a*1=a or sqrt(0)=0 or ln(e^a)=a
				Equation *x = _left;
				Equation *y = _right;
				Equation _O{0}, *O = &_O;
				Equation _I{1}, *I = &_I;
				bool changed = false;
				
#define FUNCBASE(N, ...) ({ \
					Equation* cases[][(N) + 1] = {__VA_ARGS__};				\
					int size = sizeof(cases)/sizeof(Equation*)/((N) + 1);	\
					changed = analyze<(N)>(cases, size, vars);				\
				})
				
				if (unary()) {
					Equation* vars[1] = {x};
#define FUNC1(name, inv, ...) 									\
					if (_op == #name) {							\
						if (*_left == #inv && _left->unary()) {	\
							_left->movefrom(LEFT);				\
							movefrom(LEFT);						\
							changed = true;						\
						} else FUNCBASE(1, __VA_ARGS__);		\
					} else
#include "scripts/simplify.dat"
						;
				} else {
					Equation* vars[2] = {x, y};
#define FUNC2(name, ...) 												\
					if (_op == #name) FUNCBASE(2, __VA_ARGS__); else
#include "scripts/simplify.dat"
						;
				}
				
				if (changed)
					return simplify();
			}
			
#ifdef ADVANCED_MODE
			if (*_left == CONST && *_right == CONST) { // calculate binary: 42+11=53
				double x = _left->_num;
				double y = _right->_num;
				bool changed = false;
				
#define FUNC2(name, cname) 				\
				if (_op == #name) {		\
					_num = x cname y;	\
					changed = true;		\
				} else
#include "scripts/calculate.dat"
						;
				
				if (changed) {
					_type = CONST;
					delete _left;
					delete _right;
					return;
				}
			}
			
			if (*_left == CONST && unary()) { // calculate unary: no examples
				double x = _left->_num;
				bool changed = false;
				
#define FUNC1(name, cname) 						\
					if (_op == #name) {			\
						_num = cname(x);		\
						changed = true;			\
					} else
#include "scripts/calculate.dat"
						;
				
				if (changed) {
					_type = CONST;
					delete _left;
					delete _right;
					return;
				}
			}
			
			if (*_left == _op && associative(_op)) { // most common case for associativity: (a+b)+c=a+(b+c)
				std::swap(_left, _right);
				std::swap(_left, _right->_left);
				std::swap(_right->_left, _right->_left);
				return simplify();
			}
			
			if (((*_left == OP && *_right != OP) || (*_left == X && *_right == CONST)) && commutative(_op)) { // case for commutativity: f(x,y)+a=a+f(x,y) or x+42=42+x
				std::swap(_left, _right);
				return simplify();
			}
			
			if (*_left != OP && *_right == _op && _left->_type == _right->_left->_type && associative(_op)) { // combination for comm+assoc: x+(x+b)=(x+x)+b=b+(x+x) or 42+(11+b)=b+(42+11)
				std::swap(_left, _right->_right);
				return simplify();
			}
			
			if (*_left != CONST && *_right == _op && *_right->_left == CONST && associative(_op)) { // case for associativity: a+(42+b)=(a+42)+b=(42+a)+b=42+(a+b)
				std::swap(_left, _right->_left);
				return simplify();
			}
			
			if (*_left == *_right) { // check duplexes: x*x=x^2 or x-x=0
				bool changed = false;
				Equation x = *_left;
				
#define FUNC2(name, eq) \
				if (_op == #name) {		\
					x = Equation eq;	\
					changed = true;		\
				} else
#include "scripts/duplex.dat"
					;
					
				if (changed) {
					operator=(x);
// 					std::cout << std::endl << *this << std::endl;
					return simplify();
				}
			}
			
			/* Some specific cases, obvious though */
			/* Some copypasta next is difficult to get rid of */
			
			if (_op == "+") {
				
				if (*_left == "-" && _left->unary()) // (-a)+b=b+(-a)
					std::swap(_left, _right);
				if (*_right == "-" && _right->unary()) { // a+(-b)=b-a
					_op = "-";
					_right->movefrom(LEFT);
					return simplify();
				}
				
				// LCONV(-, +)
				if (*_left == "-" && !_left->unary()) { // (a-b)+c=a-(b-c)
					std::swap(_left, _right);
					std::swap(_left, _right->_left);
					std::swap(_right->_left, _right->_right);
					_op = "-";
					return simplify();
				}
				
				if (*_left == CONST && *_right == "-" && *_right->_left == CONST) { // 42+(11-a)=53-a
					_left->_num += _right->_left->_num;
					_op = "-";
					_right->movefrom(RIGHT);
					return simplify();
				}
				
				if (*_left == CONST && *_right == "-" && *_right->_right == CONST) { // 42+(a-11)=31-a=53+a
					_left->_num -= _right->_right->_num;
					_right->movefrom(LEFT);
					return simplify();
				}
				
			} else if (_op == "-" && !unary()) {
				
				if (*_left == 0) { // 0-b=-b
					std::swap(_left, _right);
					_right->_type = NONE;
					return simplify();
				}
				
				if (*_left == "-" && _left->unary()) { // (-a)-b=-(b+a)
					std::swap(_left->_right, _right);
					_left->_op = "+";
					return simplify();
				}
				
				if (*_right == "-" && _right->unary()) { // a-(-b)=a+b
					_op = "+";
					_right->movefrom(LEFT);
					return simplify();
				}
				
				// RCONV(-, +)
				if (*_left == "-" && !_left->unary()) { // (a-b)-c=a-(c+b)
					std::swap(_left, _right);
					std::swap(_left, _right->_left);
					_right->_op = "+";
					return simplify();
				}
				
				if (*_right == "-" && !_right->unary()) { // a-(b-c) = a+(c-b)
					std::swap(_right->_left, _right->_right);
					_op = "+";
					return simplify();
				}
				
				// RASS(+, -)
				if (*_left == "+") { // (a+b)-c=a+(b-c)
					std::swap(_left, _right);
					std::swap(_left, _right->_left);
					std::swap(_right->_left, _right->_right);
					_right->_op = "-";
					_op = "+";
					return simplify();
				}
				
			} else if (_op == "-" && unary()) {
				
				
				
			} else if (_op == "*") {
				
				// parity=odd (2x)
				if (*_left == "-" && _left->unary() && *_right == "-" && _right->unary()) { // (-a)*(-b)=a*b
					_left->movefrom(LEFT);
					_right->movefrom(LEFT);
					return simplify();
				}
				
				// parity=odd
				if (*_right == "-" && _right->unary()) // a*(-b)=(-b)*a
					std::swap(_left, _right);
				if (*_left == "-" && _left->unary()) { // (-a)*b=-(a*b)
					std::swap(_left->_right, _right);
					_op = "-";
					_left->_op = "*";
					return simplify();
				}
				
			} else if (_op == "/") {
				
				// RASS(*, /)
				if (*_left == "*") { // (a*b)/c=a*(b/c)
					std::swap(_left, _right);
					std::swap(_left, _right->_left);
					std::swap(_right->_left, _right->_right);
					_right->_op = "/";
					_op = "*";
					return simplify();
				}
				
				// RCONV(/, *)
				if (*_left == "/") { // (a/b)/c=a/(c*b)
					std::swap(_left, _right);
					std::swap(_left, _right->_left);
					_right->_op = "*";
					return simplify();
				}
				
				// parity = odd (2x)
				if (*_left == "-" && _left->unary() && *_right == "-" && _right->unary()) { // (-a)/(-b)=a/b
					_left->movefrom(LEFT);
					_right->movefrom(LEFT);
					return simplify();
				}
				
				// parity = odd
				if (*_right == "-" && _right->unary()) { // a/(-b)=(-b)/a
					std::swap(_left, _right);
					std::swap(_left->_left, _right);
				}
				if (*_left == "-" && _left->unary()) { // (-a)/b=-(a/b)
					std::swap(_left->_right, _right);
					_op = "-";
					_left->_op = "/";
					return simplify();
				}
				
			} else if (_op == "^") {
				
				// RCONV(^, *)
				if (*_left == "^") { // (a^b)^c=a^(c*b)
					std::swap(_left, _right);
					std::swap(_left, _right->_left);
					_right->_op = "*";
					return simplify();
				}
				
			}
#endif // ADVANCED_MODE
			break;
			
		default:
			throw std::runtime_error("Unexpected error: Equation::_type=" + (int)_type);
	}
}

//--------------------------------

std::string Equation::tex(int prio) const
{
	switch (_type) {
		case NONE:  return "";
		case X:     return "x";
		case CONST:
			{
				std::stringstream ss;
				ss << _num;
				return ss.str();
			}
		
		case OP:
			{
				std::string res = "";
				Equation &x = *_left, &y = *_right;
				int px = relativepriority(LEFT), py = relativepriority(RIGHT);
				
				if (unary()) {
#define FUNC1(name, cname, prio, diff, tex) \
					if (_op == #name) res = (tex); else
#include "scripts/functions.dat"
						throw std::runtime_error("Unexpected error: undefined operation \"" + _op + "\"");
				} else {
#define FUNC2(name, cname, prio, diff, tex) \
					if (_op == #name) res = (tex); else
#include "scripts/functions.dat"
						throw std::runtime_error("Unexpected error: undefined operation \"" + _op + "\"");
				}
				
				if (priority() > prio)
					return "(" + res + ")";
				return res;
			}
			
		default:
			throw std::runtime_error("Unexpected error: Equation::_type=" + (int)_type);
	}
	return "";
}


//================================| excessive access functions |================================

std::ostream& operator<<(std::ostream& os, Equation const& eq)
{
	switch (eq._type) {
		case Equation::NONE:
			break;
			
		case Equation::X:
			os << 'x';
			break;
			
		case Equation::CONST:
			os << eq._num;
			break;
			
		case Equation::OP:
			if (eq._op == "br")
				os << "[" << *eq._left << "]";
			else if (eq.unary())
				os << "{" << eq._op << " " << *eq._left << "}";
			else
				os << "(" << *eq._left << eq._op << *eq._right << ")";
			break;
			
		default:
			throw Equation::ex::UKTYPE;
	}
	return os;
}

//--------------------------------

extern Equation getEQ(std::ifstream& in);


//================================| and |================================
