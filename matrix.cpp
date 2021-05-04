#include "matrix.h"

#include <functional>
#include <limits>
#include <sstream>

Matrix::Matrix(size_t height, size_t width) :
	height(height),
	width(width),
	cells(std::vector(height * width, 0.0)) {}

Matrix::Matrix(std::initializer_list<std::initializer_list<double>> init) {
	height = init.size();
	width = 0;
	if (height > 0) {
		width = init.begin()->size();
		for (const auto& row : init) {
			if (row.size() != width) {
				throw std::invalid_argument("matrix dimension mismatch in constructor");
			}
		}
	}
	cells = std::vector(height * width, 0.0);
	size_t i = 0;
	for (const auto& row : init) {
		size_t j = 0;
		for (double val : row) {
			(*this)[{i, j}] = val;
			++j;
		}
		++i;
	}
}

size_t Matrix::get_height() const { return height; }

size_t Matrix::get_width() const { return width; }

double Matrix::operator[](std::pair<size_t, size_t> idx) const {
	return cells[width * idx.first + idx.second];
}

double& Matrix::operator[](std::pair<size_t, size_t> idx) {
	return cells[width * idx.first + idx.second];
}

Matrix operator+(const Matrix& lhs, const Matrix& rhs) {
	return lhs.apply(std::plus<double>{}, rhs);
}

Matrix operator-(const Matrix& lhs, const Matrix& rhs) {
	return lhs.apply(std::minus<double>{}, rhs);
}

Matrix operator*(const Matrix& lhs, const Matrix& rhs) {
	if (lhs.width != rhs.height) {
		throw std::invalid_argument("matrix dimension mismatch in multiplication");
	}
	size_t len = lhs.width;
	size_t height = lhs.height;
	size_t width = rhs.width;
	Matrix res(height, width);
	for (size_t i = 0; i < height; ++i) {
		for (size_t j = 0; j < width; ++j) {
			res[{i, j}] = 0.0;
			for (size_t k = 0; k < len; ++k) {
				res[{i, j}] += lhs[{i, k}] * rhs[{k, j}];
			}
		}
	}
	return res;
}

namespace {

constexpr double epsilon = std::numeric_limits<double>::epsilon();

bool zero(double x) {
	return std::abs(x) <= epsilon;
}

} // end anon

// Inversion uses the basic Gauss-Jordan elimination algorithm.
std::optional<Matrix> Matrix::inverse() const {
	if (width != height) {
		return std::nullopt;
	}
	size_t n = width;
	// Fill the augmented matrix.
	Matrix mat(n, 2*n);
	for (size_t i = 0; i < n; ++i) {
		for (size_t j = 0; j < n; ++j) {
			mat[{i, j}] = (*this)[{i, j}];
		}
	}
	for (size_t i = 0; i < n; ++i) {
		mat[{i, n + i}] = 1.0;
	}
	// Do the elimination.
	for (size_t k = 0; k < n; ++k) {
		size_t max = k;
		for (size_t i = k+1; i < n; ++i) {
			if (std::abs(mat[{i, k}]) > std::abs(mat[{max, k}])) {
				max = i;
			}
		}
		if (zero(mat[{max, k}])) {
			return std::nullopt;
		}
		if (max != k) {
			for (size_t i = k; i < 2*n; ++i) {
				std::swap(mat[{k, i}], mat[{max, i}]);
			}
		}
		for (size_t i = 0; i < n; ++i) {
			if (i == k) {
				continue;
			}
			double ratio = mat[{i, k}] / mat[{k, k}];
			mat[{i, k}] = 0.0;
			for (size_t j = k+1; j < 2*n; ++j) {
				mat[{i, j}] -= ratio * mat[{k, j}];
			}
		}
	}
	for (size_t k = 0; k < n; ++k) {
		for (size_t i = n; i < 2*n; ++i) {
			mat[{k, i}] /= mat[{k, k}];
		}
		mat[{k, k}] = 1.0;
	}
	Matrix res(n, n);
	for (size_t i = 0; i < n; ++i) {
		for (size_t j = 0; j < n; ++j) {
			res[{i, j}] = mat[{i, n + j}];
		}
	}
	return res;
}

std::string Matrix::show() const {
	std::ostringstream out;
	for (size_t i = 0; i < height; ++i) {
		for (size_t j = 0; j < width; ++j) {
			out << std::to_string((*this)[{i, j}]);
			if (j + 1 < width) {
				out << '\t';
			}
		}
		out << std::endl;
	}
	return out.str();
}
