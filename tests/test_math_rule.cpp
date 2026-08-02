#include <iostream>
#include <string_view>

#include "rules/math_rule.hpp"

namespace
{

int failures = 0;

void Expect(
    bool condition,
    const char* message
)
{
    if (!condition)
    {
        std::cerr
            << "[FAIL] "
            << message
            << '\n';

        ++failures;
    }
}

void ExpectMatch(
    const qualix::rules::MathRule& rule,
    std::string_view input,
    std::string_view expected,
    const char* message
)
{
    const auto match =
        rule.Match(input, 0);

    Expect(
        match.Matched(),
        message
    );

    if (!match.Matched())
        return;

    Expect(
        match.type ==
            qualix::rules::RuleType::Math,
        "Matched type is Math"
    );

    Expect(
        match.View(input) == expected,
        message
    );
}

void ExpectNoMatch(
    const qualix::rules::MathRule& rule,
    std::string_view input,
    const char* message
)
{
    Expect(
        !rule.Match(input, 0).Matched(),
        message
    );
}

} // namespace

int main()
{
    using qualix::rules::MathRule;

    const MathRule rule;

    ExpectMatch(
        rule,
        "1+2",
        "1+2",
        "Basic addition"
    );

    ExpectMatch(
        rule,
        "10-5",
        "10-5",
        "Basic subtraction"
    );

    ExpectMatch(
        rule,
        "3*4",
        "3*4",
        "Basic multiplication"
    );

    ExpectMatch(
        rule,
        "10/2",
        "10/2",
        "Basic division"
    );

    ExpectMatch(
        rule,
        "2^8",
        "2^8",
        "Exponent expression"
    );

    ExpectMatch(
        rule,
        "(1+2)*3",
        "(1+2)*3",
        "Parenthesized expression"
    );

    ExpectMatch(
        rule,
        "x+1",
        "x+1",
        "Variable expression"
    );

    ExpectMatch(
        rule,
        "a+b",
        "a+b",
        "Identifier expression"
    );

    ExpectMatch(
        rule,
        "x = 10",
        "x = 10",
        "Assignment expression"
    );

    ExpectMatch(
        rule,
        "3.14*2",
        "3.14*2",
        "Decimal expression"
    );

    ExpectNoMatch(
        rule,
        " 1+2",
        "Leading whitespace rejected"
    );

    ExpectNoMatch(
        rule,
        "123",
        "Plain number is not Math"
    );

    ExpectNoMatch(
        rule,
        "hello",
        "Plain word is not Math"
    );

    ExpectNoMatch(
        rule,
        "3.14",
        "Plain decimal is not Math"
    );

    ExpectNoMatch(
        rule,
        "(1+2",
        "Unclosed parenthesis rejected"
    );

    ExpectNoMatch(
        rule,
        "1++2",
        "Double plus rejected"
    );

    ExpectNoMatch(
        rule,
        "1**2",
        "Double multiply rejected"
    );

    ExpectNoMatch(
        rule,
        "1//2",
        "Double division rejected"
    );

    ExpectNoMatch(
        rule,
        "x+",
        "Missing right operand rejected"
    );

    ExpectNoMatch(
        rule,
        "x=",
        "Incomplete assignment rejected"
    );

    ExpectNoMatch(
        rule,
        "1+*2",
        "Mixed consecutive operators rejected"
    );

    ExpectNoMatch(
        rule,
        "1 2+3",
        "Adjacent operands separated by space rejected"
    );

    ExpectNoMatch(
        rule,
        "9876543210",
        "Phone-like number rejected"
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

    if (failures != 0)
    {
        std::cerr
            << failures
            << " test(s) failed\n";

        return 1;
    }

    std::cout
        << "All MathRule tests passed\n";

    return 0;
}
