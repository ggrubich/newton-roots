#include "matrix.h"

#include "gtest/gtest.h"

void expect_matrix_eq(const Matrix& actual, const Matrix& expected) {
	ASSERT_EQ(actual.get_height(), expected.get_height()) << "matrix height";
	ASSERT_EQ(actual.get_width(), expected.get_width()) << "matrix width";
	for (size_t i = 0; i < expected.get_height(); ++i) {
		for (size_t j = 0; j < expected.get_width(); ++j) {
			auto actual_ = actual[{i, j}];
			auto expected_ = expected[{i, j}];
			EXPECT_DOUBLE_EQ(actual_, expected_)
				<< "matrix cell at (" << i << ", " << j << ")";
		}
	}
}

void expect_matrix_near(const Matrix& actual, const Matrix& expected, double error) {
	ASSERT_EQ(actual.get_height(), expected.get_height()) << "matrix height";
	ASSERT_EQ(actual.get_width(), expected.get_width()) << "matrix width";
	for (size_t i = 0; i < expected.get_height(); ++i) {
		for (size_t j = 0; j < expected.get_width(); ++j) {
			auto actual_ = actual[{i, j}];
			auto expected_ = expected[{i, j}];
			EXPECT_NEAR(actual_, expected_, error)
				<< "matrix cell at (" << i << ", " << j << ")";
		}
	}
}

TEST(MatrixTest, Add) {
	Matrix lhs = {
		{0.6, 0.3, 0.1},
		{0.2, 0.7, 0.1},
		{0.1, 0.1, 0.8},
	};
	Matrix rhs = {
		{1.3, 0.1, 0.9},
		{-1.5, 0.6, -0.1},
		{0.7, 0.0, 1.0},
	};
	Matrix expected = {
		{1.9, 0.4, 1.0},
		{-1.3, 1.3, 0.0},
		{0.8, 0.1, 1.8},
	};
	Matrix actual = lhs + rhs;
	expect_matrix_eq(actual, expected);
}

TEST(MatrixTest, Sub) {
	Matrix lhs = {
		{0.6, 0.3, 0.1},
		{0.2, 0.7, 0.1},
		{0.1, 0.1, 0.8},
	};
	Matrix rhs = {
		{1.3, 0.1, 0.9},
		{-1.5, 0.6, -0.1},
		{0.7, 0.0, 1.0},
	};
	Matrix expected = {
		{-0.7, 0.2, -0.8},
		{1.7, 0.1, 0.2},
		{-0.6, 0.1, -0.2},
	};
	Matrix actual = lhs - rhs;
	expect_matrix_eq(actual, expected);
}

TEST(MatrixTest, Mul) {
	Matrix lhs = {
		{0.6, 0.3, 0.1},
		{0.2, 0.7, 0.1},
		{0.1, 0.1, 0.8},
	};
	Matrix rhs = {
		{2.0, 3.0},
		{-0.5, 0.5},
		{0.0, 0.1},
	};
	Matrix expected = {
		{1.05, 1.96},
		{0.05, 0.96},
		{0.15, 0.43},
	};
	Matrix actual = lhs * rhs;
	expect_matrix_near(actual, expected, 1.0e-14);
}

TEST(MatrixTest, InvertBad) {
	Matrix mat = {
		{0.6, 0.3, 0.1},
		{200.0, 700.0, 100.0},
		{-0.6, -2.1, -0.3},
	};
	auto actual = mat.inverse();
	EXPECT_FALSE(actual) << "matrix inverse existence";
}

TEST(MatrixTest, Invert1) {
	Matrix mat = {{2.1}};
	double expected = 1.0 / 2.1;
	auto inv = mat.inverse();
	ASSERT_TRUE(inv) << "matrix inverse existence";
	double actual = (*inv)[{0, 0}];
	EXPECT_DOUBLE_EQ(actual, expected) << "1x1 matrix inverse";
}

TEST(MatrixTest, Invert3) {
	Matrix mat = {
		{0.6, 0.3, 0.1},
		{0.2, 0.7, 0.1},
		{0.1, 0.1, 0.8},
	};
	Matrix expected = {
		{ 1.96429,  -0.821429, -0.142857},
		{-0.535714,  1.67857,  -0.142857},
		{-0.178571, -0.107143,  1.28571 },
	};
	auto actual = mat.inverse();
	ASSERT_TRUE(actual) << "matrix inverse existence";
	expect_matrix_near(*actual, expected, 0.000005);
}
