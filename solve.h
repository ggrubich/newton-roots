#ifndef ROOTS_SOLVE_H
#define ROOTS_SOLVE_H

#include "expr.h"

#include <limits>
#include <vector>

using Binding = std::pair<std::string, double>;

struct Constraints {
	// Minimal number of iterations.
	size_t min_iters = 1;
	// Maximal number of iterations.
	size_t max_iters = 100;
	// Value used for absolute comparisons of numbers.
	// Numbers x and y are considered absolutely equal when
	//   abs(x - y) <= abs_epsilon
	double abs_epsilon = std::numeric_limits<double>::epsilon();
	// Value used for relative comparisons of numbers.
	// Numbers x and y are considered relatively equal when
	//   abs(x - y) <= max(abs(x), abs(y)) * rel_epsilon
	double rel_epsilon = std::numeric_limits<double>::epsilon();
};

struct Solution {
	// Number of iterations performed.
	size_t iters;
	// Maximal absolute difference between variable from the last iteration
	// and its counterpart from the preceding iteration.
	// If we assume that convergence is quadratic or better, max_diff is
	// solution's upper error bound.
	double max_diff;
	// Computed varibles.
	std::vector<Binding> vars;
};

// Solves a system of functions using Newton's method, starting with the given
// initial solution. Throws MathError on failure.
// The algorithm will successfully terminate iff the following conditions are met:
//  - The number of iterations is in range [min_iters, max_iters].
//  - Variables computed in the last iteration are approximately equal to their
//    counterparts from the previous iteration. Equality is determined according
//    to supplied epsilon values.
Solution
solve(const std::vector<Expr>& funcs, const std::vector<Binding>& init, Constraints constr);

#endif // ROOTS_SOLVE_H
