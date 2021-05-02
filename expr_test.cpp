#include "expr.h"

#include <cmath>

#include "gtest/gtest.h"

using namespace std::literals;

TEST(ExprTest, Variables) {
	auto expr = Expr("x") + 2*Expr("foo bar") / (13.0 + Expr("x")).cos() - (-Expr("z"));
	Expr::Vars expected = {"x", "z", "foo bar"};
	auto actual = expr.variables();
	EXPECT_EQ(actual, expected) << "free variables in expression";
}

void compare_eval(
		Expr expr,
		const std::function<double(double, double)>& fun,
		const std::vector<std::pair<double, double>>& points)
{
	for (const auto& p : points) {
		auto x = p.first;
		auto y = p.second;
		auto actual = expr.eval({{"x", x}, {"y", y}});
		auto expected = fun(x, y);
		EXPECT_FLOAT_EQ(actual, expected) << "evaluating at x = " << x << ", y = " << y;
	}
}

TEST(ExprTest, EvalBasic) {
	auto expr = -Expr(1.0) * ((2.0 * Expr("x") + (Expr("x") + Expr("y")) / 3.0) - 0.5 * Expr("x"));
	auto fun = [](double x, double y) {
		return -1.0 * ((2.0 * x + (x + y) / 3.0) - 0.5 * x);
	};
	std::vector<std::pair<double, double>> points = {
		{1.0, 1.0},
		{-13.13, 12.37},
		{104.1, 149.09},
		{-31.0, -20.0},
		{0.5, 0.0},
	};
	compare_eval(expr, fun, points);
}

TEST(ExprTest, EvalTrig) {
	auto expr = 100*Expr("x").sin().sin() * (Expr("y") + Expr("x")).cos();
	auto fun = [](double x, double y) {
		return 100*std::sin(std::sin(x)) * std::cos(y + x);
	};
	std::vector<std::pair<double, double>> points = {
		{0.25, -0.75},
		{1.23, 3.21},
		{13.13, -3.32},
		{39.19, -3.18},
		{1000.10, -193148.13},
	};
	compare_eval(expr, fun, points);
}

TEST(ExprTest, EvalEuler) {
	auto expr = Expr("x").exp() - Expr("y").ln().exp().exp();
	auto fun = [](double x, double y) {
		return std::exp(x) - std::exp(std::exp(std::log(y)));
	};
	std::vector<std::pair<double, double>> points = {
		{1.0, 0.79},
		{19.13, 31.13},
		{-193.13, 0.123},
		{82.01, 80.91},
		{0.11314, 0.4181},
	};
	compare_eval(expr, fun, points);
}

void compare_diff(
		Expr expr,
		const std::function<double(double, double)>& fun,
		const std::vector<std::pair<double, double>>& points)
{
	for (const auto& p : points) {
		auto x = p.first;
		auto y = p.second;
		auto actual = expr.diff("x", {{"x", x}, {"y", y}});
		auto expected = fun(x, y);
		EXPECT_FLOAT_EQ(actual, expected) << "differentiating over x at x = " << x << ", y = " << y;
	}
}

TEST(ExprTest, DiffBasic) {
	auto expr = ((Expr("x") + Expr("y")) / Expr("y")) * Expr("x")
		- (Expr("x") * Expr("x") * Expr("y"));
	auto fun = [](double x, double y) {
		return (2*x / y) + 1 - (2*x*y);
	};
	std::vector<std::pair<double, double>> points = {
		{13.24, 0.5},
		{-90.5, 12.2},
		{0.25, 0.0012},
		{833.31, 8301.31},
		{91.149, -9.013},
	};
	compare_diff(expr, fun, points);
}

TEST(ExprTest, DiffTrig) {
	auto expr = (Expr("x")*Expr("x")).sin().cos();
	auto fun = [](double x, double y) {
		return -std::sin(std::sin(x*x)) * std::cos(x*x) * 2*x;
	};
	std::vector<std::pair<double, double>> points = {
		{0.0, 0.0},
		{12.34, 0.0},
		{-21.14, 0.0},
		{0.0441, 0.0},
		{381.194, 0.0},
	};
	compare_diff(expr, fun, points);
}

TEST(ExprTest, DiffEuler) {
	auto expr = (3.0 * Expr("x") * Expr("y")).exp()
		+ (Expr("x") * Expr("y") * Expr("y")).ln();
	auto fun = [](double x, double y) {
		return std::exp(3*x*y) * (3*y) + (1/x);
	};
	std::vector<std::pair<double, double>> points = {
		{1.0, -1.0},
		{0.13, 2.31},
		{3.1, 4.51},
		{10.11, -0.02},
		{5.041, -13.3},
	};
	compare_diff(expr, fun, points);
}

TEST(ExprTest, ParseBinary) {
	auto input = "1 + 2 * 3 - 4 / 5 / 6 - 2"s;
	double expected = 1.0 + (2.0 * 3.0) - (4.0 / 5.0) / 6.0 - 2.0;
	double actual = Expr::parse(input).eval({});
	EXPECT_FLOAT_EQ(actual, expected) << input;
}

TEST(ExprTest, ParseUnary) {
	auto input = "-2*2 - -sin(3 + cos 4) - exp ln 12 * -1"s;
	double expected = -4.0 + std::sin(3.0 + std::cos(4)) + 12.0;
	double actual = Expr::parse(input).eval({});
	EXPECT_FLOAT_EQ(actual, expected) << input;
}

TEST(ExprTest, ParseWhitespace) {
	auto input = "  1-2+3\t* 2\n\t  /2 \n"s;
	double expected = 1.0 - 2.0 + 3.0 * 2.0 / 2.0;
	double actual = Expr::parse(input).eval({});
	EXPECT_FLOAT_EQ(actual, expected) << input;
}

TEST(ExprTest, ParseVariables) {
	auto input = " xy / sin123 + x_1-a1b2 + x.2"s;
	Expr::Vars expected = {"xy", "sin123", "x_1", "a1b2", "x.2"};
	auto actual = Expr::parse(input).variables();
	EXPECT_EQ(actual, expected) << "variables in " << input;
}

TEST(ExprTest, ParseNumbers) {
	auto input = "0.75 + 0.0 + 12.34 - 10"s;
	double expected = 0.75 + 12.34 - 10.0;
	double actual = Expr::parse(input).eval({});
	EXPECT_FLOAT_EQ(actual, expected) << input;
}

TEST(ExprTest, ParseParenthesis) {
	auto input = " ( (1 + 3) * (((( 1 - 2 ) * 3)) - sin(((1)-2))) )"s;
	double expected = (1.0 + 3.0) * (((1.0-2.0) * 3.0) - std::sin(1.0-2.0));
	double actual = Expr::parse(input).eval({});
	EXPECT_FLOAT_EQ(actual, expected) << input;
}
