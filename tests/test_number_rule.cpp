#include <string>
#include <string_view>

#include "rules/number_rule.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::rules;
using namespace qualix::test;

namespace
{

void ExpectNumber(
    std::string_view input,
    std::string_view expected,
    std::string_view name
)
{
    NumberRule rule;

    const auto match =
        rule.Match(input, 0);

    Expect(
        match.Matched(),
        name
    );

    if (!match.Matched())
        return;

    Expect(
        match.type ==
            RuleType::Number,
        "Number match type"
    );

    Expect(
        match.View(input) ==
            expected,
        "Number match text"
    );
}

void ExpectReject(
    std::string_view input,
    std::string_view name
)
{
    NumberRule rule;

    Expect(
        !rule.Match(input, 0).Matched(),
        name
    );
}

} // namespace

int main()
{
    ExpectNumber(
        "123",
        "123",
        "Integer"
    );

    ExpectNumber(
        "+123",
        "+123",
        "Positive signed integer"
    );

    ExpectNumber(
        "-123",
        "-123",
        "Negative signed integer"
    );

    ExpectNumber(
        "12.50",
        "12.50",
        "Decimal"
    );

    ExpectNumber(
        ".75",
        ".75",
        "Leading dot decimal"
    );

    ExpectNumber(
        "1.",
        "1.",
        "Trailing dot decimal"
    );

    ExpectNumber(
        "1,000",
        "1,000",
        "Grouped thousands"
    );

    ExpectNumber(
        "1,000,000.50",
        "1,000,000.50",
        "Grouped decimal"
    );

    ExpectNumber(
        "1_000_000",
        "1_000_000",
        "Underscore separated integer"
    );

    ExpectNumber(
        "6.022e23",
        "6.022e23",
        "Scientific notation"
    );

    ExpectNumber(
        "1.5E-10",
        "1.5E-10",
        "Signed exponent"
    );

    ExpectNumber(
        "-2.5e+8",
        "-2.5e+8",
        "Signed scientific number"
    );

    ExpectNumber(
        "0xFF",
        "0xFF",
        "Hexadecimal"
    );

    ExpectNumber(
        "0XCAFE",
        "0XCAFE",
        "Uppercase hexadecimal prefix"
    );

    ExpectNumber(
        "0xCA_FE",
        "0xCA_FE",
        "Hexadecimal underscore"
    );

    ExpectNumber(
        "0b1010",
        "0b1010",
        "Binary"
    );

    ExpectNumber(
        "0b1010_0101",
        "0b1010_0101",
        "Binary underscore"
    );

    ExpectNumber(
        "0o755",
        "0o755",
        "Octal"
    );

    ExpectNumber(
        "123!",
        "123",
        "Factorial operator excluded"
    );

    ExpectNumber(
        "50%",
        "50",
        "Percent sign excluded"
    );

    ExpectNumber(
        "123,",
        "123",
        "Trailing comma excluded"
    );

    ExpectNumber(
        "123)",
        "123",
        "Closing parenthesis excluded"
    );

    ExpectReject(
        ".",
        "Dot only rejected"
    );

    ExpectReject(
        "+",
        "Plus only rejected"
    );

    ExpectReject(
        "-",
        "Minus only rejected"
    );

    ExpectReject(
        "0x",
        "Hex prefix without digits rejected"
    );

    ExpectReject(
        "0b",
        "Binary prefix without digits rejected"
    );

    ExpectReject(
        "0o",
        "Octal prefix without digits rejected"
    );

    ExpectReject(
        "0xGG",
        "Invalid hexadecimal rejected"
    );

    ExpectReject(
        "0b102",
        "Invalid binary rejected"
    );

    ExpectReject(
        "123abc",
        "Number inside identifier rejected"
    );

    ExpectReject(
        "abc123",
        "Identifier prefix rejected"
    );

    ExpectReject(
        "1__000",
        "Double underscore rejected"
    );

    NumberRule rule;

    Expect(
        rule.Type() ==
            RuleType::Number,
        "Number rule reports type"
    );


    ExpectNumber(
        "1,000",
        "1,000",
        "International grouped thousand"
    );

    ExpectNumber(
        "12,345",
        "12,345",
        "International grouped number"
    );

    ExpectNumber(
        "1,234,567",
        "1,234,567",
        "International multi grouping"
    );

    ExpectNumber(
        "1,25,000",
        "1,25,000",
        "Indian lakh grouping"
    );

    ExpectNumber(
        "12,34,567",
        "12,34,567",
        "Indian lakh grouping two digit prefix"
    );

    ExpectNumber(
        "1,23,45,678",
        "1,23,45,678",
        "Indian crore grouping"
    );

    ExpectNumber(
        "12,34,56,789",
        "12,34,56,789",
        "Indian crore grouping two digit prefix"
    );

    return Summary();
}
