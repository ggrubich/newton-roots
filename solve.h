#ifndef ROOTS_SOLVE_H
#define ROOTS_SOLVE_H

#include "expr.h"

#include <vector>

using Binding = std::pair<std::string, double>;

// Solves a system of functions using Newton's method, starting with the given
// initial solution.
// Throws MathError on failure.
std::vector<Binding>
solve(const std::vector<Expr>& funcs, const std::vector<Binding>& init, size_t max_iters);

#endif // ROOTS_SOLVE_H
