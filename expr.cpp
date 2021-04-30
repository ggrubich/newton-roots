#include "expr.h"

#include "common.h"

#include <cmath>
#include <functional>
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

struct Float {
	double v;

	Float();
	Float(double v);

	Float sin() const;
	Float cos() const;
	Float ln() const;
	Float exp() const;
};

Float::Float() : v(0) {}
Float::Float(double v) : v(v) {}

Float operator+(Float x, Float y) { return Float(x.v + y.v); }
Float operator-(Float x, Float y) { return Float(x.v - y.v); }
Float operator*(Float x, Float y) { return Float(x.v * y.v); }
Float operator/(Float x, Float y) { return Float(x.v / y.v); }

Float operator-(Float x) { return Float(-x.v); }
Float Float::sin() const { return Float(std::sin(v)); }
Float Float::cos() const { return Float(std::cos(v)); }
Float Float::ln() const { return Float(std::log(v)); }
Float Float::exp() const { return Float(std::exp(v)); }

// Dual numbers for forward mode automatic differentiation.
struct Dual {
	double v;
	double dv;

	Dual();
	Dual(double c);
	Dual(double v, double dv);

	Dual sin() const;
	Dual cos() const;
	Dual ln() const;
	Dual exp() const;
};

Dual::Dual() : v(0.0), dv(0.0) {}
Dual::Dual(double c) : v(c), dv(0.0) {}
Dual::Dual(double v, double dv) : v(v), dv(dv) {}

Dual operator+(Dual x, Dual y) {
	return Dual(x.v + y.v, x.dv + y.dv);
}
Dual operator-(Dual x, Dual y) {
	return Dual(x.v - y.v, x.dv - y.dv);
}
Dual operator*(Dual x, Dual y) {
	return Dual(x.v * y.v, (x.dv * y.v) + (x.v * y.dv));
}
Dual operator/(Dual x, Dual y) {
	return Dual(x.v / y.v, ((x.dv * y.v) - (x.v * y.dv)) / (y.v * y.v));
}

Dual operator-(Dual x) {
	return Dual(-x.v, -x.dv);
}
Dual Dual::sin() const {
	return Dual(std::sin(v), std::cos(v) * dv);
}
Dual Dual::cos() const {
	return Dual(std::cos(v), -std::sin(v) * dv);
}
Dual Dual::ln() const {
	return Dual(std::log(v), dv / v);
}
Dual Dual::exp() const {
	return Dual(std::exp(v), std::exp(v) * dv);
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
			}
			return out;
		},
		[&](const Unary& un) {
			auto arg = eval_rec(*un.arg, seed);
			Num out;
			switch (un.type) {
			case UnaryOp::Neg: out = -arg; break;
			case UnaryOp::Sin: out = arg.sin(); break;
			case UnaryOp::Cos: out = arg.cos(); break;
			case UnaryOp::Ln:  out = arg.ln(); break;
			case UnaryOp::Exp: out = arg.exp(); break;
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
			throw std::domain_error("undefined variable " + var);
		}
		return Float(it->second);
	});
	return out.v;
}

double Expr::diff(const std::string& x, const Env& env) const {
	auto out = eval_rec<Dual>(*this, [&](const std::string& var) {
		auto it = env.find(var);
		if (it == env.end()) {
			throw std::domain_error("undefined variable " + var);
		}
		auto v = it->second;
		auto dv = var == x ? 1.0 : 0.0;
		return Dual(v, dv);
	});
	return out.dv;
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
			case UnaryOp::Neg: op = "-"; break;
			case UnaryOp::Sin: op = "sin"; break;
			case UnaryOp::Cos: op = "cos"; break;
			case UnaryOp::Ln:  op = "ln"; break;
			case UnaryOp::Exp: op = "exp"; break;
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
