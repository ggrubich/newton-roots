#include "solve.h"

#include "common.h"
#include "matrix.h"

#include <cmath>
#include <stdexcept>

namespace {

bool equals(double a, double b, double abs_eps, double rel_eps) {
	double delta = std::abs(a - b);
	double scale = std::max(std::abs(a), std::abs(b));
	return delta <= abs_eps || delta <= scale * rel_eps;
}

bool matrix_equals(const Matrix& a, const Matrix& b, Constraints constr) {
	double abs_eps = constr.abs_epsilon;
	double rel_eps = constr.rel_epsilon;
	for (size_t i = 0; i < a.get_height(); ++i) {
		for (size_t j = 0; j < a.get_width(); ++j) {
			if (!equals(a[{i, j}], b[{i, j}], abs_eps, rel_eps)) {
				return false;
			}
		}
	}
	return true;
}

} // end anon

Solution
solve(const std::vector<Expr>& funcs, const std::vector<Binding>& init, Constraints constr) {
	Matrix x0(init.size(), 1, [&](size_t i, size_t j) {
		return init[i].second;
	});
	for (size_t k = 1; k <= constr.max_iters; ++k) {
		Expr::Env env;
		for (size_t i = 0; i < x0.get_height(); ++i) {
			env[init[i].first] = x0[{i, 0}];
		}
		Matrix jac(funcs.size(), init.size(), [&](size_t i, size_t j) {
			return funcs[i].diff(init[j].first, env);
		});
		Matrix y(funcs.size(), 1, [&](size_t i, size_t j) {
			return funcs[i].eval(env);
		});
		auto jac_inv = jac.inverse();
		if (!jac_inv) {
			throw MathError("division impossible; algorithm stuck at iteration " + std::to_string(k));
		}
		Matrix x1 = x0 - (*jac_inv) * y;
		if (k >= constr.min_iters && matrix_equals(x0, x1, constr)) {
			Solution res;
			res.iters = k;
			res.max_diff = 0.0;
			for (size_t i = 0; i < init.size(); ++i) {
				res.max_diff = std::max(res.max_diff,
						std::abs(x1[{i, 0}] - x0[{i, 0}]));
				res.vars.emplace_back(init[i].first, x1[{i, 0}]);
			}
			return res;
		}
		x0 = std::move(x1);
	}
	throw MathError("no solution found for given constraints");
}
