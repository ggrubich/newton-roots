#ifndef ROOTS_EXPR_H
#define ROOTS_EXPR_H

#include <optional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>

// Tree structure representing a mathematical expression.
class Expr {
public:
	struct Const {
		double val;

		Const(double val);
	};

	struct Var {
		std::shared_ptr<const std::string> name;

		Var(std::string name);
	};

	enum class BinaryOp {
		Add,
		Sub,
		Mul,
		Div,
		Pow,
	};

	struct Binary {
		BinaryOp type;
		std::shared_ptr<const Expr> lhs;
		std::shared_ptr<const Expr> rhs;

		Binary(BinaryOp type, Expr x, Expr y);
	};

	enum class UnaryOp {
		Neg,
		Sin,
		Cos,
		Ln,
		Exp,
		Sqrt,
	};

	struct Unary {
		UnaryOp type;
		std::shared_ptr<const Expr> arg;

		Unary(UnaryOp type, Expr x);
	};

	using Value = std::variant<Const, Var, Binary, Unary>;

	Value value;

	Expr();
	Expr(Value val);
	// Constructs a constant.
	Expr(double x);
	// Construct a variable.
	Expr(std::string var);

	friend Expr operator+(Expr lhs, Expr rhs);
	friend Expr operator-(Expr lhs, Expr rhs);
	friend Expr operator*(Expr lhs, Expr rhs);
	friend Expr operator/(Expr lhs, Expr rhs);
	Expr pow(Expr rhs) const;

	friend Expr operator-(Expr x);
	Expr sin() const;
	Expr cos() const;
	Expr ln() const;
	Expr exp() const;
	Expr sqrt() const;

	using Vars = std::unordered_set<std::string>;

private:
	void variables_rec(Vars& vars) const;
public:

	// Finds free variables in the expression.
	Vars variables() const;

	using Env = std::unordered_map<std::string, double>;

	// Evaluates the expression in the given environment.
	// Throws MathError on failure.
	double eval(const Env& env) const;

	// Partially differentiates the expresstion in relation to given variable x
	// in the given environment.
	// Throws MathError on failure.
	double diff(const std::string& x, const Env& env) const;

private:
	void show_rec(std::string& buf) const;
public:

	// Returns a string representation of the expression.
	std::string show() const;

	// Parses an expression from string.
	// On invalid input throws a ParseError.
	static Expr parse(const std::string& input);
};

#endif // ROOTS_EXPR_H
