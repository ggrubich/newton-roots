#ifndef ROOTS_COMMON_H
#define ROOTS_COMMON_H

#include <stdexcept>

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

class ParseError : public std::runtime_error {
	using runtime_error::runtime_error;
};

#endif // ROOTS_COMMON_H
