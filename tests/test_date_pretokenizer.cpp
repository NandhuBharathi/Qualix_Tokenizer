#include <iostream>
#include <string>
#include <string_view>

#include "pretokenizer/pretokenizer.hpp"
#include "pretokenizer/span_type.hpp"

using namespace qualix;
using namespace qualix::pretokenizer;

namespace {

int tests_run = 0;
int tests_passed = 0;
int tests_failed = 0;

void Expect(
    bool condition,
    std::string_view name
)
{
    ++tests_run;

    if (condition)
    {
        ++tests_passed;
        std::cout
            << "[PASS] "
            << name
            << "\n";
    }
    else
    {
        ++tests_failed;
        std::cout
            << "[FAIL] "
            << name
            << "\n";
    }
}

void ExpectDate(
    std::string_view text,
    std::string_view expected,
    std::string_view name
)
{
    const auto result =
        PreTokenizer::Split(text);

    Expect(
        result.Ok(),
        name
    );

    if (result.Failed())
        return;

    const auto& spans =
        result.Value();

    Expect(
        spans.size() == 1,
        "Date one span"
    );

    if (spans.size() != 1)
        return;

    const auto& span =
        spans.front();

    Expect(
        span.View(text) == expected,
        "Date text preserved"
    );

    Expect(
        span.type == SpanType::Date,
        "Date span type"
    );

    Expect(
        span.Protected(),
        "Date protected"
    );
}

} // namespace

int main()
{
    ExpectDate(
        "12/3/25",
        "12/3/25",
        "Short date protected"
    );

    ExpectDate(
        "12/03/2025",
        "12/03/2025",
        "Full slash date protected"
    );

    ExpectDate(
        "2025-03-12",
        "2025-03-12",
        "ISO date protected"
    );

    ExpectDate(
        "12.03.2025",
        "12.03.2025",
        "Dot date protected"
    );

    ExpectDate(
        "29/02/2024",
        "29/02/2024",
        "Leap date protected"
    );

    {
        const std::string text =
            "Today is 12/3/25.";

        const auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "Date sentence split"
        );

        if (result.Ok())
        {
            bool found = false;

            for (const auto& span :
                 result.Value())
            {
                if (span.View(text) ==
                    "12/3/25")
                {
                    found =
                        span.type ==
                            SpanType::Date &&
                        span.Protected();
                }
            }

            Expect(
                found,
                "Sentence date protected"
            );
        }
    }

    {
        const std::string text =
            "12/03/2025.";

        const auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "Date punctuation split"
        );

        if (result.Ok())
        {
            const auto& spans =
                result.Value();

            Expect(
                spans.size() == 2,
                "Date punctuation span count"
            );

            if (spans.size() == 2)
            {
                Expect(
                    spans[0].View(text) ==
                        "12/03/2025",
                    "Date before punctuation"
                );

                Expect(
                    spans[0].type ==
                        SpanType::Date,
                    "Date before punctuation type"
                );

                Expect(
                    spans[0].Protected(),
                    "Date before punctuation protected"
                );

                Expect(
                    spans[1].View(text) == ".",
                    "Date punctuation separate"
                );

                Expect(
                    spans[1].type ==
                        SpanType::Punctuation,
                    "Date punctuation type"
                );
            }
        }
    }

    {
        const std::string text =
            "123 50% 12/3/25 "
            "₹500 user@example.com "
            "https://example.com";

        const auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "All rules coexist"
        );

        if (result.Ok())
        {
            bool number_found = false;
            bool percentage_found = false;
            bool date_found = false;
            bool currency_found = false;
            bool email_found = false;
            bool url_found = false;

            for (const auto& span :
                 result.Value())
            {
                const auto view =
                    span.View(text);

                if (view == "123")
                {
                    number_found =
                        span.type ==
                            SpanType::Number &&
                        span.Protected();
                }
                else if (view == "50%")
                {
                    percentage_found =
                        span.type ==
                            SpanType::Percentage &&
                        span.Protected();
                }
                else if (view == "12/3/25")
                {
                    date_found =
                        span.type ==
                            SpanType::Date &&
                        span.Protected();
                }
                else if (view == "₹500")
                {
                    currency_found =
                        span.type ==
                            SpanType::Currency &&
                        span.Protected();
                }
                else if (
                    view ==
                    "user@example.com")
                {
                    email_found =
                        span.type ==
                            SpanType::Email &&
                        span.Protected();
                }
                else if (
                    view ==
                    "https://example.com")
                {
                    url_found =
                        span.type ==
                            SpanType::Url &&
                        span.Protected();
                }
            }

            Expect(
                number_found,
                "Number coexist"
            );

            Expect(
                percentage_found,
                "Percentage coexist"
            );

            Expect(
                date_found,
                "Date coexist"
            );

            Expect(
                currency_found,
                "Currency coexist"
            );

            Expect(
                email_found,
                "Email coexist"
            );

            Expect(
                url_found,
                "URL coexist"
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
        << tests_failed
        << "\n"
        << "================================\n";

    return tests_failed == 0
        ? 0
        : 1;
}
