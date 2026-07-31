#pragma once

#include <cstdlib>
#include <iostream>
#include <string_view>

namespace qualix::test
{

inline int tests_run = 0;
inline int tests_passed = 0;
inline int tests_failed = 0;

inline void Expect(bool condition, std::string_view name)
{
    ++tests_run;

    if (condition)
    {
        ++tests_passed;
        std::cout << "[PASS] " << name << '\n';
    }
    else
    {
        ++tests_failed;
        std::cout << "[FAIL] " << name << '\n';
    }
}

inline int Summary()
{
    std::cout << "\n================================\n";
    std::cout << "Tests Run    : " << tests_run << '\n';
    std::cout << "Tests Passed : " << tests_passed << '\n';
    std::cout << "Tests Failed : " << tests_failed << '\n';
    std::cout << "================================\n";

    return tests_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

} // namespace qualix::test
