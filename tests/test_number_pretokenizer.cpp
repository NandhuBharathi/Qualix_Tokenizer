#include <iostream>
#include <string_view>

#include "pretokenizer/pretokenizer.hpp"

using namespace qualix;
using namespace qualix::pretokenizer;

namespace
{

usize tests_run = 0;
usize tests_passed = 0;

void Expect(
    bool condition,
    std::string_view name
)
{
    ++tests_run;

    if (condition)
    {
        ++tests_passed;
        std::cout << "[PASS] " << name << "\n";
    }
    else
    {
        std::cout << "[FAIL] " << name << "\n";
    }
}

void ExpectProtectedNumber(
    std::string_view input,
    std::string_view expected,
    std::string_view name
)
{
    auto result =
        PreTokenizer::Split(input);

    Expect(
        !result.Failed(),
        name
    );

    if (result.Failed())
        return;

    const auto& spans =
        result.Value();

    Expect(
        spans.size() == 1,
        "Number one span"
    );

    if (spans.size() != 1)
        return;

    Expect(
        spans[0].View(input) == expected,
        "Number text preserved"
    );

    if (spans[0].type != SpanType::Number)
    {
        std::cout
            << "[DEBUG] input=["
            << input
            << "] actual_type="
            << ToString(spans[0].type)
            << "\n";
    }

    Expect(
        spans[0].type == SpanType::Number,
        "Number span type"
    );

    Expect(
        spans[0].Protected(),
        "Number protected"
    );
}

} // namespace

int main()
{
    ExpectProtectedNumber(
        "123",
        "123",
        "Integer protected"
    );

    ExpectProtectedNumber(
        "-123",
        "-123",
        "Signed integer protected"
    );

    ExpectProtectedNumber(
        "12.75",
        "12.75",
        "Decimal protected"
    );

    ExpectProtectedNumber(
        "1,234.56",
        "1,234.56",
        "Grouped decimal protected"
    );

    ExpectProtectedNumber(
        "1.2e10",
        "1.2e10",
        "Scientific notation protected"
    );

    ExpectProtectedNumber(
        "0xFF",
        "0xFF",
        "Hexadecimal protected"
    );

    ExpectProtectedNumber(
        "0b1010",
        "0b1010",
        "Binary protected"
    );

    ExpectProtectedNumber(
        "0o755",
        "0o755",
        "Octal protected"
    );

    {
        constexpr std::string_view input =
            "Value -12.5 test";

        auto result =
            PreTokenizer::Split(input);

        Expect(
            !result.Failed(),
            "Number sentence split"
        );

        if (!result.Failed())
        {
            const auto& spans =
                result.Value();

            Expect(
                spans.size() == 5,
                "Number sentence span count"
            );

            if (spans.size() == 5)
            {
                Expect(
                    spans[0].View(input) == "Value" &&
                    spans[0].type == SpanType::Word,
                    "Word before number"
                );

                Expect(
                    spans[1].type ==
                        SpanType::Whitespace,
                    "Whitespace before number"
                );

                Expect(
                    spans[2].View(input) == "-12.5",
                    "Signed decimal preserved"
                );

                Expect(
                    spans[2].type ==
                        SpanType::Number,
                    "Signed decimal type"
                );

                Expect(
                    spans[2].Protected(),
                    "Signed decimal protected"
                );

                Expect(
                    spans[4].View(input) == "test" &&
                    spans[4].type == SpanType::Word,
                    "Word after number"
                );
            }
        }
    }

    {
        constexpr std::string_view input =
            "5!";

        auto result =
            PreTokenizer::Split(input);

        Expect(
            !result.Failed(),
            "Factorial boundary split"
        );

        if (!result.Failed())
        {
            const auto& spans =
                result.Value();

            Expect(
                spans.size() == 2,
                "Factorial boundary span count"
            );

            if (spans.size() == 2)
            {
                Expect(
                    spans[0].View(input) == "5" &&
                    spans[0].type == SpanType::Number &&
                    spans[0].Protected(),
                    "Factorial number protected"
                );

                Expect(
                    spans[1].View(input) == "!" &&
                    spans[1].type ==
                        SpanType::Punctuation &&
                    !spans[1].Protected(),
                    "Factorial operator separate"
                );
            }
        }
    }

    {
        constexpr std::string_view input =
            "50%";

        auto result =
            PreTokenizer::Split(input);

        Expect(
            !result.Failed(),
            "Percentage boundary split"
        );

        if (!result.Failed())
        {
            const auto& spans =
                result.Value();

            Expect(
                spans.size() == 1,
                "Percentage boundary span count"
            );

            if (spans.size() == 1)
            {
                Expect(
                    spans[0].View(input) == "50%" &&
                    spans[0].type ==
                        SpanType::Percentage &&
                    spans[0].Protected(),
                    "Percentage protected"
                );
            }
        }
    }

    {
        constexpr std::string_view input =
            "abc123";

        auto result =
            PreTokenizer::Split(input);

        Expect(
            !result.Failed(),
            "Identifier-like text split"
        );

        if (!result.Failed())
        {
            const auto& spans =
                result.Value();

            bool protected_number = false;

            for (const auto& span : spans)
            {
                if (span.type == SpanType::Number &&
                    span.Protected())
                {
                    protected_number = true;
                }
            }

            Expect(
                !protected_number,
                "Number inside identifier not protected"
            );
        }
    }

    {
        constexpr std::string_view input =
            "42 test@example.com https://example.com";

        auto result =
            PreTokenizer::Split(input);

        Expect(
            !result.Failed(),
            "Number email URL coexist"
        );

        if (!result.Failed())
        {
            bool number = false;
            bool email = false;
            bool url = false;

            for (const auto& span :
                 result.Value())
            {
                if (span.type == SpanType::Number &&
                    span.Protected())
                {
                    number = true;
                }

                if (span.type == SpanType::Email &&
                    span.Protected())
                {
                    email = true;
                }

                if (span.type == SpanType::Url &&
                    span.Protected())
                {
                    url = true;
                }
            }

            Expect(
                number,
                "Protected number coexist"
            );

            Expect(
                email,
                "Protected email coexist"
            );

            Expect(
                url,
                "Protected URL coexist"
            );
        }
    }

    std::cout
        << "\n================================\n"
        << "Tests Run    : "
        << tests_run
        << "\n"
        << "Tests Passed : "
        << tests_passed
        << "\n"
        << "Tests Failed : "
        << tests_run - tests_passed
        << "\n"
        << "================================\n";

    return
        tests_run == tests_passed
            ? 0
            : 1;
}
