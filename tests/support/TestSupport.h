#pragma once

#include <cstdlib>
#include <iostream>

namespace HGGFTests {

inline void require(bool condition,
                    const char* expression,
                    const char* file,
                    int line) {
    if (condition)
        return;

    std::cerr
        << "FAIL: " << expression
        << " at " << file
        << ":" << line
        << "\n";

    std::exit(EXIT_FAILURE);
}

} // namespace HGGFTests

#define HGGF_REQUIRE(expression) \
    ::HGGFTests::require( \
        static_cast<bool>(expression), \
        #expression, \
        __FILE__, \
        __LINE__)
