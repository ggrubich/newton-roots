#ifndef ROOTS_COMMON_H
#define ROOTS_COMMON_H

#include <stdexcept>

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

class BaseError : public std::runtime_error {
	using runtime_error::runtime_error;
};

class MathError : public BaseError {
	using BaseError::BaseError;
};

class ParseError : public BaseError {
	using BaseError::BaseError;
};

#endif // ROOTS_COMMON_H
