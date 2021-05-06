#include "solve.h"

#include "matrix.h"

#include <stdexcept>

std::vector<Binding>
solve(const std::vector<Expr>& funcs, const std::vector<Binding>& init, size_t max_iters) {
	Matrix x(init.size(), 1);
	for (size_t i = 0; i < init.size(); ++i) {
		x[{i, 0}] = init[i].second;
	}
	for (size_t k = 0; k < max_iters; ++k) {
		Expr::Env env;
		for (size_t i = 0; i < x.get_height(); ++i) {
			env[init[i].first] = x[{i, 0}];
		}
		Matrix jac(funcs.size(), init.size(), [&](size_t i, size_t j) {
			return funcs[i].diff(init[j].first, env);
		});
		Matrix y(funcs.size(), 1, [&](size_t i, size_t j) {
			return funcs[i].eval(env);
		});
		auto jac_inv = jac.inverse();
		if (!jac_inv) {
			throw std::runtime_error("division impossible; algorithm stuck at iteration " + std::to_string(k));
		}
		x = x - (*jac_inv) * y;
	}
	std::vector<Binding> result;
	for (size_t i = 0; i < init.size(); ++i) {
		result.emplace_back(init[i].first, x[{i, 0}]);
	}
	return result;
}
