#ifndef ROOTS_COMMON_H
#define ROOTS_COMMON_H

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

#endif // ROOTS_COMMON_H
