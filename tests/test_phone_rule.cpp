#include <iostream>
#include <string_view>

#include "rules/phone_rule.hpp"

using qualix::rules::PhoneRule;

namespace
{

int tests_run = 0;
int tests_passed = 0;
int tests_failed = 0;

void Expect(
    bool condition,
    const char* name
)
{
    ++tests_run;

    if (condition)
    {
        ++tests_passed;
        std::cout
            << "[PASS] "
            << name
            << '\n';
    }
    else
    {
        ++tests_failed;
        std::cout
            << "[FAIL] "
            << name
            << '\n';
    }
}

void ExpectMatch(
    const PhoneRule& rule,
    std::string_view input,
    std::string_view expected,
    const char* name
)
{
    const auto match =
        rule.Match(input, 0);

    const bool ok =
        match.Matched() &&
        match.View(input) == expected;

    Expect(ok, name);
}

void ExpectNoMatch(
    const PhoneRule& rule,
    std::string_view input,
    const char* name
)
{
    Expect(
        !rule.Match(input, 0).Matched(),
        name
    );
}

} // namespace

int main()
{
    PhoneRule rule;

    ExpectMatch(
        rule,
        "+91 9876543210",
        "+91 9876543210",
        "Indian international phone"
    );

    ExpectMatch(
        rule,
        "+919876543210",
        "+919876543210",
        "Indian compact international phone"
    );

    ExpectMatch(
        rule,
        "9876543210",
        "9876543210",
        "Indian mobile phone"
    );

    ExpectMatch(
        rule,
        "98765 43210",
        "98765 43210",
        "Indian grouped mobile phone"
    );

    ExpectMatch(
        rule,
        "98765-43210",
        "98765-43210",
        "Indian hyphenated mobile phone"
    );

    ExpectMatch(
        rule,
        "+1 202 555 0123",
        "+1 202 555 0123",
        "US international phone"
    );

    ExpectMatch(
        rule,
        "+44 20 7946 0958",
        "+44 20 7946 0958",
        "UK international phone"
    );

    ExpectMatch(
        rule,
        "(202) 555-0123",
        "(202) 555-0123",
        "Parenthesized phone"
    );

    ExpectNoMatch(
        rule,
        "123",
        "Short number rejected"
    );

    ExpectNoMatch(
        rule,
        "12345",
        "Five digit number rejected"
    );

    ExpectNoMatch(
        rule,
        "12.75",
        "Decimal rejected"
    );

    ExpectNoMatch(
        rule,
        "12/03/2026",
        "Date rejected"
    );

    ExpectNoMatch(
        rule,
        "2026-08-02",
        "ISO date rejected"
    );

    ExpectNoMatch(
        rule,
        "10kg",
        "Measurement rejected"
    );

    ExpectNoMatch(
        rule,
        "50%",
        "Percentage rejected"
    );

    ExpectNoMatch(
        rule,
        "0xFF",
        "Hex number rejected"
    );

    std::cout
        << "\n================================\n"
        << "Tests Run    : "
        << tests_run
        << '\n'
        << "Tests Passed : "
        << tests_passed
        << '\n'
        << "Tests Failed : "
        << tests_failed
        << '\n'
        << "================================\n";

    return tests_failed == 0
        ? 0
        : 1;
}
