#include <string_view>

#include "rules/measurement_rule.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::rules;
using namespace qualix::test;

namespace
{

void ExpectMeasurement(
    std::string_view text,
    std::string_view expected,
    const char* name
)
{
    MeasurementRule rule;

    const auto match =
        rule.Match(text, 0);

    Expect(
        match.Matched(),
        name
    );

    if (!match.Matched())
        return;

    Expect(
        match.type ==
            RuleType::Measurement,
        "Measurement match type"
    );

    Expect(
        match.View(text) ==
            expected,
        "Measurement match text"
    );
}

void ExpectRejected(
    std::string_view text,
    const char* name
)
{
    MeasurementRule rule;

    const auto match =
        rule.Match(text, 0);

    Expect(
        !match.Matched(),
        name
    );
}

} // namespace

int main()
{
    // ---------------------------------------------------------
    // Length
    // ---------------------------------------------------------

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
        "1.5km",
        "1.5km",
        "Decimal measurement"
    );

    ExpectMeasurement(
        "-12.5 cm",
        "-12.5 cm",
        "Signed measurement"
    );

    // ---------------------------------------------------------
    // Temperature
    // ---------------------------------------------------------

    ExpectMeasurement(
        "-20°C",
        "-20°C",
        "Celsius measurement"
    );

    ExpectMeasurement(
        "98.6°F",
        "98.6°F",
        "Fahrenheit measurement"
    );

    // ---------------------------------------------------------
    // Frequency / electrical
    // ---------------------------------------------------------

    ExpectMeasurement(
        "2.4GHz",
        "2.4GHz",
        "Frequency measurement"
    );

    ExpectMeasurement(
        "220V",
        "220V",
        "Voltage measurement"
    );

    ExpectMeasurement(
        "5kW",
        "5kW",
        "Power measurement"
    );

    // ---------------------------------------------------------
    // Data
    // ---------------------------------------------------------

    ExpectMeasurement(
        "500 MB",
        "500 MB",
        "Data measurement"
    );

    ExpectMeasurement(
        "1.5GB",
        "1.5GB",
        "Decimal data measurement"
    );

    // ---------------------------------------------------------
    // Duration unit
    // ---------------------------------------------------------

    ExpectMeasurement(
        "5ms",
        "5ms",
        "Millisecond measurement"
    );

    // ---------------------------------------------------------
    // Trailing punctuation
    // ---------------------------------------------------------

    ExpectMeasurement(
        "25kg,",
        "25kg",
        "Trailing punctuation excluded"
    );

    // ---------------------------------------------------------
    // Invalid / incomplete
    // ---------------------------------------------------------

    ExpectRejected(
        "100",
        "Bare number rejected"
    );

    ExpectRejected(
        "kg",
        "Bare unit rejected"
    );

    ExpectRejected(
        "10unknown",
        "Unknown unit rejected"
    );

    ExpectRejected(
        "10kgabc",
        "Unit identifier continuation rejected"
    );

    {
        MeasurementRule rule;

        Expect(
            rule.Type() ==
                RuleType::Measurement,
            "Measurement rule reports type"
        );
    }

    return Summary();
}
