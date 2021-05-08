#include "expr.h"

#include "common.h"

#include <cfenv>
#include <cmath>
#include <functional>
#include <sstream>
#include <stdexcept>

Expr::Const::Const(double val) : val(val) {}

Expr::Var::Var(std::string name) :
	name(std::make_shared<std::string>(std::move(name))) {}

Expr::Binary::Binary(BinaryOp type, Expr x, Expr y) :
	type(type),
	lhs(std::make_shared<Expr>(std::move(x))),
	rhs(std::make_shared<Expr>(std::move(y))) {}

Expr::Unary::Unary(UnaryOp type, Expr x) :
	type(type),
	arg(std::make_shared<Expr>(std::move(x))) {}

Expr::Expr() : value(Const(0.0)) {}
Expr::Expr(Value val) : value(std::move(val)) {}
Expr::Expr(double x) : value(Const(x)) {}
Expr::Expr(std::string var) : value(Var(std::move(var))) {}

namespace {

using Const = Expr::Const;
using Var = Expr::Var;
using BinaryOp = Expr::BinaryOp;
using Binary = Expr::Binary;
using UnaryOp = Expr::UnaryOp;
using Unary = Expr::Unary;
using Env = Expr::Env;

} // end anon

Expr operator+(Expr lhs, Expr rhs) {
	return Expr(Binary(BinaryOp::Add, std::move(lhs), std::move(rhs)));
}

Expr operator-(Expr lhs, Expr rhs) {
	return Expr(Binary(BinaryOp::Sub, std::move(lhs), std::move(rhs)));
}

Expr operator*(Expr lhs, Expr rhs) {
	return Expr(Binary(BinaryOp::Mul, std::move(lhs), std::move(rhs)));
}

Expr operator/(Expr lhs, Expr rhs) {
	return Expr(Binary(BinaryOp::Div, std::move(lhs), std::move(rhs)));
}

Expr Expr::pow(Expr rhs) const {
	return Expr(Binary(BinaryOp::Pow, *this, std::move(rhs)));
}

Expr operator-(Expr x) {
	return Expr(Unary(UnaryOp::Neg, std::move(x)));
}

Expr Expr::sin() const {
	return Expr(Unary(UnaryOp::Sin, *this));
}

Expr Expr::cos() const {
	return Expr(Unary(UnaryOp::Cos, *this));
}

Expr Expr::ln() const {
	return Expr(Unary(UnaryOp::Ln, *this));
}

Expr Expr::exp() const {
	return Expr(Unary(UnaryOp::Exp, *this));
}

Expr Expr::sqrt() const {
	return Expr(Unary(UnaryOp::Sqrt, *this));
}

void Expr::variables_rec(Vars& vars) const {
	std::visit(overloaded {
		[](const Const&) {},
		[&](const Var& var) {
			vars.insert(*var.name);
		},
		[&](const Binary& bin) {
			bin.lhs->variables_rec(vars);
			bin.rhs->variables_rec(vars);
		},
		[&](const Unary& un) {
			un.arg->variables_rec(vars);
		},
	}, value);
}

Expr::Vars Expr::variables() const {
	Vars vars;
	variables_rec(vars);
	return vars;
}

namespace {

template<typename Fun>
double checked(Fun fun) {
	std::feclearexcept(FE_ALL_EXCEPT);
	double y = fun();
	if (std::fetestexcept(FE_DIVBYZERO | FE_INVALID)) {
		throw MathError("domain error");
	}
	return y;
}

double ediv(double x, double y) {
	return checked([=]() { return x / y; });
}

double epow(double x, double y) {
	return checked([=]() { return std::pow(x ,y); });
}

double elog(double x) {
	return checked([=]() { return std::log(x); });
}

double esqrt(double x) {
	return checked([=]() { return std::sqrt(x); });
}

struct Float {
	double val;

	Float();
	Float(double val);

	Float pow(Float y) const;

	Float sin() const;
	Float cos() const;
	Float ln() const;
	Float exp() const;
	Float sqrt() const;
};

Float::Float() : val(0) {}
Float::Float(double val) : val(val) {}

Float operator+(Float x, Float y) { return Float(x.val + y.val); }
Float operator-(Float x, Float y) { return Float(x.val - y.val); }
Float operator*(Float x, Float y) { return Float(x.val * y.val); }
Float operator/(Float x, Float y) { return Float(ediv(x.val, y.val)); }
Float Float::pow(Float y) const { return Float(epow(val, y.val)); }

Float operator-(Float x) { return Float(-x.val); }
Float Float::sin() const { return Float(std::sin(val)); }
Float Float::cos() const { return Float(std::cos(val)); }
Float Float::ln() const { return Float(elog(val)); }
Float Float::exp() const { return Float(std::exp(val)); }
Float Float::sqrt() const { return Float(esqrt(val)); }

// Dual number for forward mode automatic differentiation.
// It contains a value (val), its derivative (deriv) and a boolean flag (cons)
// indicating whether the number is a constant or not.
struct Dual {
	double val;
	double deriv;
	bool cons;

	Dual();
	Dual(double val);
	Dual(double val, double deriv, bool cons);

	Dual pow(Dual y) const;

	Dual sin() const;
	Dual cos() const;
	Dual ln() const;
	Dual exp() const;
	Dual sqrt() const;
};

Dual::Dual() : Dual(0.0, 0.0, true) {}
Dual::Dual(double val) : Dual(val, 0.0, true) {}
Dual::Dual(double val, double deriv, bool cons) : val(val), deriv(deriv), cons(cons) {}

Dual operator+(Dual x, Dual y) {
	return Dual(x.val + y.val, x.deriv + y.deriv, x.cons && y.cons);
}

Dual operator-(Dual x, Dual y) {
	return Dual(x.val - y.val, x.deriv - y.deriv, x.cons && y.cons);
}

Dual operator*(Dual x, Dual y) {
	return Dual(x.val * y.val,
			(x.deriv * y.val) + (x.val * y.deriv),
			x.cons && y.cons);
}

Dual operator/(Dual x, Dual y) {
	return Dual(ediv(x.val, y.val),
			ediv((x.deriv * y.val) - (x.val * y.deriv), y.val * y.val),
			x.cons && y.cons);
}

Dual Dual::pow(Dual y) const {
	auto x = *this;
	double out;
	if (y.cons) {
		// For constant exponents we use the basic (x^k)' = k * x^(k-1) formula
		// with an exception for k = 1.
		if (y.val == 1.0) {
			out = 1.0;
		}
		else {
			out = y.val * epow(x.val, y.val - 1.0) * x.deriv;
		}
	}
	else {
		// For functional exponents we use the generalized power rule.
		out = epow(x.val, y.val) *
			(y.deriv * elog(x.val) + ediv(x.deriv * y.val, x.val));
	}
	return Dual(epow(x.val, y.val), out, x.cons && y.cons);
}

Dual operator-(Dual x) {
	return Dual(-x.val, -x.deriv, x.cons);
}

Dual Dual::sin() const {
	return Dual(std::sin(val), std::cos(val) * deriv, cons);
}

Dual Dual::cos() const {
	return Dual(std::cos(val), -std::sin(val) * deriv, cons);
}

Dual Dual::ln() const {
	return Dual(elog(val), ediv(deriv, val), cons);
}

Dual Dual::exp() const {
	return Dual(std::exp(val), std::exp(val) * deriv, cons);
}

Dual Dual::sqrt() const {
	return Dual(esqrt(val), ediv(deriv, 2 * esqrt(val)), cons);
}

template<typename Num>
Num eval_rec(const Expr& expr, const std::function<Num(const std::string&)>& seed) {
	return std::visit(overloaded {
		[](const Const& c) {
			return Num(c.val);
		},
		[&](const Var& var) {
			return seed(*var.name);
		},
		[&](const Binary& bin) {
			auto lhs = eval_rec(*bin.lhs, seed);
			auto rhs = eval_rec(*bin.rhs, seed);
			Num out;
			switch (bin.type) {
			case BinaryOp::Add: out = lhs + rhs; break;
			case BinaryOp::Sub: out = lhs - rhs; break;
			case BinaryOp::Mul: out = lhs * rhs; break;
			case BinaryOp::Div: out = lhs / rhs; break;
			case BinaryOp::Pow: out = lhs.pow(rhs); break;
			}
			return out;
		},
		[&](const Unary& un) {
			auto arg = eval_rec(*un.arg, seed);
			Num out;
			switch (un.type) {
			case UnaryOp::Neg:  out = -arg; break;
			case UnaryOp::Sin:  out = arg.sin(); break;
			case UnaryOp::Cos:  out = arg.cos(); break;
			case UnaryOp::Ln:   out = arg.ln(); break;
			case UnaryOp::Exp:  out = arg.exp(); break;
			case UnaryOp::Sqrt: out = arg.sqrt(); break;
			}
			return out;
		},
	}, expr.value);
}

} // end anon

double Expr::eval(const Env& env) const {
	auto out = eval_rec<Float>(*this, [&](const std::string& var) {
		auto it = env.find(var);
		if (it == env.end()) {
			throw MathError("undefined variable " + var);
		}
		return Float(it->second);
	});
	return out.val;
}

double Expr::diff(const std::string& x, const Env& env) const {
	auto out = eval_rec<Dual>(*this, [&](const std::string& var) {
		auto it = env.find(var);
		if (it == env.end()) {
			throw MathError("undefined variable " + var);
		}
		auto v = it->second;
		if (var == x) {
			return Dual(v, 1.0, false);
		}
		else {
			return Dual(v, 0.0, true);
		}
	});
	return out.deriv;
}

void Expr::show_rec(std::string& buf) const {
	return std::visit(overloaded {
		[&](const Const& c) {
			buf.append(std::to_string(c.val));
		},
		[&](const Var& var) {
			buf.append(*var.name);
		},
		[&](const Binary& bin) {
			buf.append("(");
			bin.lhs->show_rec(buf);
			std::string op;
			switch (bin.type) {
			case BinaryOp::Add: op = "+"; break;
			case BinaryOp::Sub: op = "-"; break;
			case BinaryOp::Mul: op = "*"; break;
			case BinaryOp::Div: op = "/"; break;
			case BinaryOp::Pow: op = "^"; break;
			}
			buf.append(" ");
			buf.append(op);
			buf.append(" ");
			bin.rhs->show_rec(buf);
			buf.append(")");
		},
		[&](const Unary& un) {
			std::string op;
			switch (un.type) {
			case UnaryOp::Neg:  op = "-"; break;
			case UnaryOp::Sin:  op = "sin"; break;
			case UnaryOp::Cos:  op = "cos"; break;
			case UnaryOp::Ln:   op = "ln"; break;
			case UnaryOp::Exp:  op = "exp"; break;
			case UnaryOp::Sqrt: op = "sqrt"; break;
			}
			if (std::isalpha(op[0])) {
				buf.append(op);
				buf.append("(");
				un.arg->show_rec(buf);
				buf.append(")");
			}
			else {
				buf.append(op);
				un.arg->show_rec(buf);
			}
		},
	}, value);
}

std::string Expr::show() const {
	std::string buf;
	show_rec(buf);
	return buf;
}

namespace {

// Expression parsing using the precedence climbing method.

enum class TokenType {
	Ident,
	Op,
	LParen,
	RParen,
	Number,
	Eof,
};

struct Token {
	TokenType type;
	std::string text;

	std::string show() const;
};

std::string Token::show() const {
	std::string out;
	switch (type) {
	case TokenType::Ident:
		out = "identifier '" + text + "'";
		break;
	case TokenType::Op:
		out = "operator '" + text + "'";
		break;
	case TokenType::LParen:
		out = "left parenthesis";
		break;
	case TokenType::RParen:
		out = "right parenthesis";
		break;
	case TokenType::Number:
		out = "number '" + text + "'";
		break;
	case TokenType::Eof:
		out = "eof";
		break;
	}
	return out;
}

class Tokenizer {
private:
	static constexpr std::string_view operators = "=+-*/^";

	std::string::const_iterator iter;
	std::string::const_iterator end;
	Token token;

public:
	Tokenizer(const std::string& input);

private:
	char get() const;
	void consume();
	void skip();
	void read_ident();
	void read_number();

public:
	void read();
	const Token* operator->() const;
};

Tokenizer::Tokenizer(const std::string& input) :
	iter(input.begin()),
	end(input.end()) {}

char Tokenizer::get() const {
	return iter == end ? 0 : *iter;
}

void Tokenizer::consume() {
	token.text.push_back(*iter);
	++iter;
}

void Tokenizer::skip() {
	++iter;
}

void Tokenizer::read_ident() {
	token.type = TokenType::Ident;
	consume();
	while (std::isalnum(get()) || get() == '_' || get() == '.') {
		consume();
	}
}

void Tokenizer::read_number() {
	token.type = TokenType::Number;
	consume();
	while (std::isdigit(get())) {
		consume();
	}
	if (get() == '.') {
		do {
			consume();
		} while (std::isdigit(get()));
	}
}

void Tokenizer::read() {
	token.text.clear();
	while (std::isspace(get())) {
		skip();
	}
	if (iter == end) {
		token.type = TokenType::Eof;
	}
	else if (std::isalpha(get())) {
		read_ident();
	}
	else if (operators.find(get()) != std::string_view::npos) {
		token.type = TokenType::Op;
		consume();
	}
	else if (get() == '(') {
		token.type = TokenType::LParen;
		consume();
	}
	else if (get() == ')') {
		token.type = TokenType::RParen;
		consume();
	}
	else if (std::isdigit(get())) {
		read_number();
	}
	else {
		std::ostringstream msg;
		msg << "unrecognized symbol '" << get() << "'";
		throw ParseError(msg.str());
	}
}

const Token* Tokenizer::operator->() const {
	return &token;
}

struct BinaryDef {
	BinaryOp op;
	int prec;
	bool rassoc;
};

const std::unordered_map<std::string, BinaryDef> binary_ops = {
	// We treat "=" as a low precedence subtraction. This is a bit of a hack,
	// but it should be okay for our purposes.
	{"=", {BinaryOp::Sub, 0, false}},
	{"+", {BinaryOp::Add, 1, false}},
	{"-", {BinaryOp::Sub, 1, false}},
	{"*", {BinaryOp::Mul, 2, false}},
	{"/", {BinaryOp::Div, 2, false}},
	{"^", {BinaryOp::Pow, 3, true}},
};

struct UnaryDef {
	UnaryOp op;
	int prec;
	// True if the operator can be a part of a parenthesized function call.
	bool funcall;
};

const std::unordered_map<std::string, UnaryDef> unary_ops = {
	{"-",    {UnaryOp::Neg,  2, false}},
	{"sin",  {UnaryOp::Sin,  2, true}},
	{"cos",  {UnaryOp::Cos,  2, true}},
	{"ln",   {UnaryOp::Ln,   2, true}},
	{"exp",  {UnaryOp::Exp,  2, true}},
	{"sqrt", {UnaryOp::Sqrt, 2, true}},
};

Expr parse_expr(Tokenizer& tokens, int min_prec = 0);

Expr parse_atom(Tokenizer& tokens) {
	if ((tokens->type == TokenType::Op || tokens->type == TokenType::Ident) &&
			unary_ops.count(tokens->text))
	{
		auto op = unary_ops.at(tokens->text);
		tokens.read();
		Expr expr;
		// Parenthesized function calls get the highest precedence.
		// Other operators get treated according to their own precedence.
		if (op.funcall && tokens->type == TokenType::LParen) {
			expr = parse_atom(tokens);
		}
		else {
			expr = parse_expr(tokens, op.prec + 1);
		}
		return Expr(Unary(op.op, expr));
	}
	else if (tokens->type == TokenType::LParen) {
		tokens.read();
		auto expr = parse_expr(tokens);
		if (tokens->type != TokenType::RParen) {
			std::ostringstream msg;
			msg << "unexpected " << tokens->show()
				<< ", expecting right parenthesis";
			throw ParseError(msg.str());
		}
		tokens.read();
		return expr;
	}
	else if (tokens->type == TokenType::Ident) {
		auto x = Expr(Var(tokens->text));
		tokens.read();
		return x;
	}
	else if (tokens->type == TokenType::Number) {
		auto x = Expr(Const(std::stod(tokens->text)));
		tokens.read();
		return x;
	}
	else {
		std::ostringstream msg;
		msg << "unexpected " << tokens->show()
			<< ", expecting identifier, number, unary operator or parenthesized expression";
		throw ParseError(msg.str());
	}
}

Expr parse_expr(Tokenizer& tokens, int min_prec) {
	auto lhs = parse_atom(tokens);
	while (tokens->type == TokenType::Op &&
			binary_ops.count(tokens->text) &&
			binary_ops.at(tokens->text).prec >= min_prec)
	{
		auto op = binary_ops.at(tokens->text);
		tokens.read();
		auto rhs = parse_expr(tokens, op.rassoc ? op.prec : op.prec + 1);
		lhs = Expr(Binary(op.op, lhs, rhs));
	}
	return lhs;
}

} // end anon

Expr Expr::parse(const std::string& input) {
	auto tokens = Tokenizer(input);
	tokens.read();
	auto expr = parse_expr(tokens);
	if (tokens->type != TokenType::Eof) {
		std::ostringstream msg;
		msg << "unexpected " << tokens->show()
			<< ", expecting binary operator or eof";
		throw ParseError(msg.str());
	}
	return expr;
}
