#pragma once

// #define ENABLE_LOGGER_IOSTREAM

#ifdef ENABLE_LOGGER_IOSTREAM
#include <iostream>
#include <utility>
#endif

#ifdef ENABLE_LOGGER_PRINTF
#include <cstdio>
#include <utility>
#endif

namespace logger {

#ifdef ENABLE_LOGGER_IOSTREAM
namespace {

    template <typename Arg, typename... Args>
    void variadic_println(std::ostream& out, Arg&& arg, Args&&... args)
    {
        out << std::forward<Arg>(arg);
        ((out << ',' << std::forward<Args>(args)), ...);
        // out << std::endl;
    }

    inline auto ostream_impl [[maybe_unused]] = [](auto&&... args) {
        variadic_println(std::cout, std::forward<decltype(args)>(args)...);
    };

}
#endif

#ifdef ENABLE_LOGGER_PRINTF
// TODO: ma inny interfejs niz ostream, potrzebuje "%d"
namespace {
    inline auto printf_impl [[maybe_unused]] = [](auto&&... args) {
        variadic_println(std::cout, std::forward<decltype(args)>(args)...);
    };
}
#endif

void print(auto&& func [[maybe_unused]])
{
#ifdef ENABLE_LOGGER_IOSTREAM
    func(ostream_impl);
#endif
#ifdef ENABLE_LOGGER_PRINTF
    func(printf_impl);
#endif
}
}

// uzycie

//
// int some_function();
// int other_function();

// inline void test()
// {
//     int val = some_function();

//     logger::log([&](auto impl) { impl("result: %d, %d", val, other_function()); });
// }
//