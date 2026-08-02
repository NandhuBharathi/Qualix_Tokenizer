#include <string>
#include <string_view>

#include "pretokenizer/pretokenizer.hpp"
#include "pretokenizer/span_type.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::pretokenizer;
using namespace qualix::test;

namespace
{

void ExpectProtectedTime(
    std::string_view input,
    std::string_view expected,
    const char* name
)
{
    auto result = PreTokenizer::Split(input);

    Expect(
        result.Ok(),
        name
    );

    if (!result.Ok())
    {
        return;
    }

    bool found = false;

    for (const auto& span : result.Value())
    {
        if (
            span.type == SpanType::Time &&
            span.View(input) == expected &&
            span.Protected()
        )
        {
            found = true;
            break;
        }
    }

    Expect(
        found,
        "Time protected span"
    );
}

} // namespace

int main()
{
    ExpectProtectedTime(
        "10:30",
        "10:30",
        "Basic time protected"
    );

    ExpectProtectedTime(
        "23:59",
        "23:59",
        "24 hour time protected"
    );

    ExpectProtectedTime(
        "10:30:45",
        "10:30:45",
        "Seconds protected"
    );

    ExpectProtectedTime(
        "9 AM",
        "9 AM",
        "Hour AM protected"
    );

    ExpectProtectedTime(
        "9pm",
        "9pm",
        "Compact PM protected"
    );

    ExpectProtectedTime(
        "10:30 PM",
        "10:30 PM",
        "Time PM protected"
    );

    ExpectProtectedTime(
        "Meeting at 10:30 today.",
        "10:30",
        "Time in sentence"
    );

    ExpectProtectedTime(
        "நேரம் 10:30 ஆகும்.",
        "10:30",
        "Tamil context time"
    );

    ExpectProtectedTime(
        "समय 10:30 है।",
        "10:30",
        "Hindi context time"
    );

    ExpectProtectedTime(
        "時間は10:30です。",
        "10:30",
        "Japanese context time"
    );

    {
        constexpr std::string_view input =
            "12/03/2026 10:30 10kg ₹500 50%";

        auto result =
            PreTokenizer::Split(input);

        Expect(
            result.Ok(),
            "Time rules coexist"
        );

        if (result.Ok())
        {
            bool date = false;
            bool time = false;
            bool measurement = false;
            bool currency = false;
            bool percentage = false;

            for (const auto& span :
                 result.Value())
            {
                const auto view =
                    span.View(input);

                if (
                    span.type == SpanType::Date &&
                    view == "12/03/2026"
                )
                {
                    date = span.Protected();
                }

                if (
                    span.type == SpanType::Time &&
                    view == "10:30"
                )
                {
                    time = span.Protected();
                }

                if (
                    span.type == SpanType::Measurement &&
                    view == "10kg"
                )
                {
                    measurement = span.Protected();
                }

                if (
                    span.type == SpanType::Currency &&
                    view == "₹500"
                )
                {
                    currency = span.Protected();
                }

                if (
                    span.type == SpanType::Percentage &&
                    view == "50%"
                )
                {
                    percentage = span.Protected();
                }
            }

            Expect(
                date,
                "Date coexist"
            );

            Expect(
                time,
                "Time coexist"
            );

            Expect(
                measurement,
                "Measurement coexist"
            );

            Expect(
                currency,
                "Currency coexist"
            );

            Expect(
                percentage,
                "Percentage coexist"
            );
        }
    }

    return Summary();
}
