#include <iostream>
#include <string>
#include <string_view>

#include "pretokenizer/pretokenizer.hpp"
#include "pretokenizer/span_type.hpp"

using qualix::pretokenizer::PreTokenizer;
using qualix::pretokenizer::SpanType;

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

void ExpectPhone(
    std::string_view input,
    const char* name
)
{
    auto result =
        PreTokenizer::Split(input);

    Expect(
        result.Ok(),
        name
    );

    if (!result.Ok())
        return;

    const auto& spans =
        result.Value();

    Expect(
        spans.size() == 1,
        "Phone one span"
    );

    if (spans.size() != 1)
        return;

    Expect(
        spans[0].View(input) == input,
        "Phone text preserved"
    );

    Expect(
        spans[0].type ==
            SpanType::Phone,
        "Phone span type"
    );

    Expect(
        spans[0].Protected(),
        "Phone protected"
    );
}

} // namespace

int main()
{
    ExpectPhone(
        "+91 9876543210",
        "Indian international phone protected"
    );

    ExpectPhone(
        "+919876543210",
        "Indian compact phone protected"
    );

    ExpectPhone(
        "9876543210",
        "Indian mobile protected"
    );

    ExpectPhone(
        "98765 43210",
        "Grouped Indian phone protected"
    );

    ExpectPhone(
        "98765-43210",
        "Hyphenated Indian phone protected"
    );

    ExpectPhone(
        "+1 202 555 0123",
        "US phone protected"
    );

    ExpectPhone(
        "+44 20 7946 0958",
        "UK phone protected"
    );

    ExpectPhone(
        "(202) 555-0123",
        "Parenthesized phone protected"
    );

    {
        const std::string text =
            "Call +91 9876543210 today.";

        auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "Phone sentence split"
        );

        if (result.Ok())
        {
            bool found = false;

            for (const auto& span :
                 result.Value())
            {
                if (
                    span.type ==
                        SpanType::Phone &&
                    span.View(text) ==
                        "+91 9876543210"
                )
                {
                    found =
                        span.Protected();
                }
            }

            Expect(
                found,
                "Phone inside sentence protected"
            );
        }
    }

    {
        const std::string text =
            "9876543210, hello";

        auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "Phone punctuation boundary"
        );

        if (result.Ok())
        {
            const auto& spans =
                result.Value();

            bool phone = false;
            bool comma = false;

            for (const auto& span :
                 spans)
            {
                const auto view =
                    span.View(text);

                if (
                    span.type ==
                        SpanType::Phone &&
                    view ==
                        "9876543210"
                )
                {
                    phone =
                        span.Protected();
                }

                if (view == ",")
                {
                    comma =
                        !span.Protected();
                }
            }

            Expect(
                phone,
                "Phone before punctuation"
            );

            Expect(
                comma,
                "Phone punctuation separate"
            );
        }
    }

    {
        const std::string text =
            "Call +91 9876543210 on 12/03/2026 at 10:30 AM "
            "pay ₹500 for 10kg with 20% discount "
            "email user@example.com visit https://example.com";

        auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "All rules coexist with phone"
        );

        if (result.Ok())
        {
            bool phone = false;
            bool date = false;
            bool time = false;
            bool currency = false;
            bool measurement = false;
            bool percentage = false;
            bool email = false;
            bool url = false;

            for (const auto& span :
                 result.Value())
            {
                const auto view =
                    span.View(text);

                if (
                    span.type ==
                        SpanType::Phone &&
                    view ==
                        "+91 9876543210"
                )
                    phone = span.Protected();

                if (
                    span.type ==
                        SpanType::Date &&
                    view ==
                        "12/03/2026"
                )
                    date = span.Protected();

                if (
                    span.type ==
                        SpanType::Time &&
                    view ==
                        "10:30 AM"
                )
                    time = span.Protected();

                if (
                    span.type ==
                        SpanType::Currency &&
                    view ==
                        "₹500"
                )
                    currency = span.Protected();

                if (
                    span.type ==
                        SpanType::Measurement &&
                    view ==
                        "10kg"
                )
                    measurement = span.Protected();

                if (
                    span.type ==
                        SpanType::Percentage &&
                    view ==
                        "20%"
                )
                    percentage = span.Protected();

                if (
                    span.type ==
                        SpanType::Email &&
                    view ==
                        "user@example.com"
                )
                    email = span.Protected();

                if (
                    span.type ==
                        SpanType::Url &&
                    view ==
                        "https://example.com"
                )
                    url = span.Protected();
            }

            Expect(phone, "Phone coexist");
            Expect(date, "Date coexist");
            Expect(time, "Time coexist");
            Expect(currency, "Currency coexist");
            Expect(
                measurement,
                "Measurement coexist"
            );
            Expect(
                percentage,
                "Percentage coexist"
            );
            Expect(email, "Email coexist");
            Expect(url, "URL coexist");
        }
    }

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
