#include "gtest/gtest.h"

TEST(MockTest, Foo) {
	EXPECT_EQ(1, 1) << "test ok";
}

TEST(MockTest, Bar) {
	EXPECT_EQ(1, 2) << "test not ok";
}
