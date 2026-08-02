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

void ExpectMeasurement(
    const std::string& text,
    std::string_view expected,
    const char* name
)
{
    const auto result =
        PreTokenizer::Split(text);

    Expect(
        result.Ok(),
        name
    );

    if (!result.Ok())
        return;

    bool found = false;

    for (const auto& span :
         result.Value())
    {
        if (span.View(text) != expected)
            continue;

        found =
            span.type ==
                SpanType::Measurement &&
            span.Protected();

        break;
    }

    Expect(
        found,
        "Measurement protected span"
    );
}

} // namespace

int main()
{
    ExpectMeasurement(
        "10kg",
        "10kg",
        "Compact measurement"
    );

    ExpectMeasurement(
        "10 kg",
        "10 kg",
        "Spaced measurement"
    );

    ExpectMeasurement(
        "Distance is 1.5km.",
        "1.5km",
        "Measurement in sentence"
    );

    ExpectMeasurement(
        "Temperature: -20°C.",
        "-20°C",
        "Temperature protected"
    );

    ExpectMeasurement(
        "CPU runs at 2.4GHz.",
        "2.4GHz",
        "Frequency protected"
    );

    ExpectMeasurement(
        "Storage is 500 MB.",
        "500 MB",
        "Data measurement protected"
    );

    ExpectMeasurement(
        "Voltage 220V.",
        "220V",
        "Voltage protected"
    );

    ExpectMeasurement(
        "Delay 5ms.",
        "5ms",
        "Duration measurement protected"
    );

    // Multilingual surrounding text.
    // Unit itself remains the standardized symbol.

    ExpectMeasurement(
        "எடை 10 kg.",
        "10 kg",
        "Tamil context measurement"
    );

    ExpectMeasurement(
        "वजन 10 kg है।",
        "10 kg",
        "Hindi context measurement"
    );

    ExpectMeasurement(
        "重量は10kgです。",
        "10kg",
        "Japanese context measurement"
    );

    // Rule coexistence

    {
        const std::string text =
            "Pay ₹500 for 10kg at 20% discount on 12/03/2026.";

        const auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "Measurement rules coexist"
        );

        if (result.Ok())
        {
            bool currency = false;
            bool measurement = false;
            bool percentage = false;
            bool date = false;

            for (const auto& span :
                 result.Value())
            {
                const auto view =
                    span.View(text);

                if (view == "₹500")
                {
                    currency =
                        span.Protected() &&
                        span.type ==
                            SpanType::Currency;
                }
                else if (view == "10kg")
                {
                    measurement =
                        span.Protected() &&
                        span.type ==
                            SpanType::Measurement;
                }
                else if (view == "20%")
                {
                    percentage =
                        span.Protected() &&
                        span.type ==
                            SpanType::Percentage;
                }
                else if (view == "12/03/2026")
                {
                    date =
                        span.Protected() &&
                        span.type ==
                            SpanType::Date;
                }
            }

            Expect(
                currency,
                "Currency coexist"
            );

            Expect(
                measurement,
                "Measurement coexist"
            );

            Expect(
                percentage,
                "Percentage coexist"
            );

            Expect(
                date,
                "Date coexist"
            );
        }
    }

    return Summary();
}
