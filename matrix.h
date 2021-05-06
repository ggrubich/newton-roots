#ifndef ROOTS_MATRIX_H
#define ROOTS_MATRIX_H

#include <initializer_list>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

// Matrix of doubles.
class Matrix {
private:
	size_t height;
	size_t width;
	std::vector<double> cells;

public:
	// Initializes a zero matrix with given dimensions.
	Matrix(size_t height, size_t width);

	// Initializes a matrix from the given seed.
	// Seed is a callable with signature equivalent to:
	//   double seed(size_t row, size_t col)
	template<typename Seed>
	Matrix(size_t height, size_t width, const Seed& seed);

	// Initializes a literal matrix.
	Matrix(std::initializer_list<std::initializer_list<double>> init);

	size_t get_height() const;
	size_t get_width() const;

	// Matrix indexing - first argument is the row, second is the column.
	double operator[](std::pair<size_t, size_t> idx) const;
	double& operator[](std::pair<size_t, size_t> idx);

	template<typename Op>
	Matrix apply(const Op& op, const Matrix &rhs) const;

	friend Matrix operator+(const Matrix& lhs, const Matrix& rhs);
	friend Matrix operator-(const Matrix& lhs, const Matrix& rhs);
	friend Matrix operator*(const Matrix& lhs, const Matrix& rhs);

	// Inverts the matrix. If matrix is not invertible, returns nothing.
	std::optional<Matrix> inverse() const;

	// Returns the string representation of the matrix.
	std::string show() const;
};

template<typename Seed>
Matrix::Matrix(size_t height, size_t width, const Seed& seed) : Matrix(height, width) {
	for (size_t i = 0; i < height; ++i) {
		for (size_t j = 0; j < width; ++j) {
			(*this)[{i, j}] = seed(i, j);
		}
	}
}

template<typename Op>
Matrix Matrix::apply(const Op& op, const Matrix& rhs) const {
	if (width != rhs.width || height != rhs.height) {
		throw std::invalid_argument("matrix dimension mismatch in binary operation");
	}
	Matrix res(height, width);
	for (size_t i = 0; i < height; ++i) {
		for (size_t j = 0; j < width; ++j) {
			res[{i, j}] = op((*this)[{i, j}], rhs[{i, j}]);
		}
	}
	return res;
}

#endif // ROOTS_MATRIX_H
