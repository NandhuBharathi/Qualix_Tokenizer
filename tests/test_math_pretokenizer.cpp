#include <iostream>
#include <string_view>

#include "pretokenizer/pretokenizer.hpp"
#include "pretokenizer/span_type.hpp"

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

void ExpectSingle(
    std::string_view input,
    qualix::pretokenizer::SpanType expected,
    const char* message
)
{
    qualix::pretokenizer::PreTokenizer tokenizer;

    const auto result =
        tokenizer.Split(input);

    Expect(
        result.Ok(),
        message
    );

    if (!result.Ok())
        return;

    const auto& spans =
        result.Value();

    Expect(
        spans.size() == 1,
        message
    );

    if (spans.size() != 1)
        return;

    Expect(
        spans[0].type == expected,
        message
    );

    Expect(
        spans[0].View(input) == input,
        message
    );
}

} // namespace

int main()
{
    using qualix::pretokenizer::SpanType;

    // Math recognition.

    ExpectSingle(
        "1+2",
        SpanType::Math,
        "Addition recognized as Math"
    );

    ExpectSingle(
        "(1+2)*3",
        SpanType::Math,
        "Parenthesized expression recognized as Math"
    );

    ExpectSingle(
        "x+1",
        SpanType::Math,
        "Variable expression recognized as Math"
    );

    ExpectSingle(
        "x = 10",
        SpanType::Math,
        "Assignment recognized as Math"
    );

    ExpectSingle(
        "3.14*2",
        SpanType::Math,
        "Decimal expression recognized as Math"
    );

    // Rule precedence.

    ExpectSingle(
        "12/03/2026",
        SpanType::Date,
        "Date takes precedence over Math"
    );

    ExpectSingle(
        "2026-08-02",
        SpanType::Date,
        "ISO date takes precedence over Math"
    );

    ExpectSingle(
        "10kg",
        SpanType::Measurement,
        "Measurement takes precedence over Math"
    );

    ExpectSingle(
        "9876543210",
        SpanType::Phone,
        "Phone takes precedence over Number"
    );

    ExpectSingle(
        "123",
        SpanType::Number,
        "Plain number remains Number"
    );

    if (failures != 0)
    {
        std::cerr
            << failures
            << " test(s) failed\n";

        return 1;
    }

    std::cout
        << "All Math PreTokenizer tests passed\n";

    return 0;
}
