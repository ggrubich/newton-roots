#include "solve.h"

#include "common.h"

#include <cmath>

#include "gtest/gtest.h"

void expect_solution_eq(const Solution& actual,
		const std::vector<Binding>& expected)
{
	ASSERT_EQ(actual.vars.size(), expected.size()) << "solution length";
	for (size_t i = 0; i < actual.vars.size(); ++i) {
		const auto& actual_ = actual.vars[i];
		const auto& expected_ = expected[i];
		ASSERT_EQ(actual_.first, expected_.first)
			<< "variable at " << i;
		EXPECT_DOUBLE_EQ(actual_.second, expected_.second)
			<< "value of " << actual_.first;
	}
}

void expect_solution_near(const Solution& actual,
		const std::vector<Binding>& expected,
		double error)
{
	ASSERT_EQ(actual.vars.size(), expected.size()) << "solution length";
	for (size_t i = 0; i < actual.vars.size(); ++i) {
		const auto& actual_ = actual.vars[i];
		const auto& expected_ = expected[i];
		ASSERT_EQ(actual_.first, expected_.first)
			<< "variable at " << i;
		EXPECT_NEAR(actual_.second, expected_.second, error)
			<< "value of  " << actual_.first;
	}
}

const Constraints default_constr;

TEST(SolveTest, Sqrt) {
	std::vector<Expr> funcs = {Expr::parse("x^2 - 13.11")};
	std::vector<Binding> expected = {{"x", std::sqrt(13.11)}};
	auto actual = solve(funcs, {{"x", 123.0}}, default_constr);
	expect_solution_eq(actual, expected);
}

TEST(SolveTest, Linear2) {
	std::vector<Expr> funcs = {
		Expr::parse("x - y + 1"),
		Expr::parse("3*x + y - 9"),
	};
	std::vector<Binding> expected = {{"x", 2.0}, {"y", 3.0}};
	auto actual = solve(funcs, {{"x", 60}, {"y", -123}}, default_constr);
	expect_solution_eq(actual, expected);
}

TEST(SolveTest, Linear4) {
	std::vector<Expr> funcs = {
		Expr::parse("3*a0 - 3*a1 - 2*a2 - 3*a3 - 2"),
		Expr::parse("-2*a0 - 5*a2 + 4*a3 + 27"),
		Expr::parse("a0 + 3*a1 - 4*a3 + 7"),
		Expr::parse("3*a0 - a1 - 2*a2 - 3*a3 + 2"),
	};
	std::vector<Binding> expected = {
		{"a0", 3},
		{"a1", -2},
		{"a2", 5},
		{"a3", 1},
	};
	auto actual = solve(funcs,
		{{"a0", 40}, {"a1", 0}, {"a2", -10}, {"a3", 100}},
		default_constr);
	expect_solution_eq(actual, expected);
}

// Expected solutions calculated with: https://www.wolframalpha.com/input/?i=systems+of+equations+calculator

TEST(SolveTest, CircleLine) {
	std::vector<Expr> funcs = {
		Expr::parse("x^2 + y^2 - 16"),
		Expr::parse("y - (2*x - 3)"),
	};
	// Solution from the right
	std::vector<Binding> expected = {
		{"x", (6.0 + std::sqrt(71.0)) / 5.0},
		{"y", (2*std::sqrt(71) - 3.0) / 5.0},
	};
	auto actual = solve(funcs, {{"x", 100}, {"y", 130}}, default_constr);
	expect_solution_eq(actual, expected);
}

TEST(SolveTest, Exponents) {
	std::vector<Expr> funcs = {
		Expr::parse("y^2 * (exp 1)^x - 3"),
		Expr::parse("2*y*(exp 1)^x + 10*y^4"),
	};
	std::vector<Binding> expected = {
		{"x",  1.30294},
		{"y", -0.90288},
	};
	auto actual = solve(funcs, {{"x", 1}, {"y", -1}}, default_constr);
	expect_solution_near(actual, expected, 0.000005);
}

TEST(SolveTest, Nonlinear2) {
	std::vector<Expr> funcs = {
		Expr::parse("x^3 - 5*x^2 + 2*x - y + 13"),
		Expr::parse("x^3 + x^2 - 14*x - y - 19"),
	};
	std::vector<Binding> expected {{"x", 4.0}, {"y", 5.0}};
	auto actual = solve(funcs, {{"x", 8}, {"y", 10}}, default_constr);
	expect_solution_eq(actual, expected);
}

TEST(SolveTest, Nonlinear3) {
	std::vector<Expr> funcs = {
		Expr::parse("x^3 - 5*x^2 + 2*x - y + 13"),
		Expr::parse("x^3 + x^2 - 14*x - y - 19"),
		Expr::parse("2*y - x*z - 1"),
	};
	std::vector<Binding> expected {{"x", 4.0}, {"y", 5.0}, {"z", 9.0/4.0}};
	auto actual = solve(funcs, {{"x", 20}, {"y", 5}, {"z", 0}}, default_constr);
	expect_solution_eq(actual, expected);
}

TEST(SolveTest, Failure) {
	std::vector<Expr> funcs = {
		Expr::parse("y = x - 3"),
		Expr::parse("y = x^2"),
	};
	EXPECT_THROW(solve(funcs, {{"x", 1}, {"y", 1}}, default_constr), MathError);
}
