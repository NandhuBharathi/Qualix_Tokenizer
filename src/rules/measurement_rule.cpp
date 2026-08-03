#include "rules/measurement_rule.hpp"
#include "rules/numeric_scanner.hpp"

#include <string_view>

namespace qualix::rules
{

namespace
{


NumericScanPolicy MeasurementNumericPolicy() noexcept
{
    NumericScanPolicy policy{};

    policy.allow_based_numbers=false;
    policy.allow_grouping=false;
    policy.allow_underscore=false;
    policy.allow_leading_dot=false;
    policy.allow_trailing_dot=false;
    policy.allow_sign=true;
    policy.allow_exponent=true;

    return policy;
}


constexpr std::string_view kUnits[] = {
    // Length
    "mm", "cm", "dm", "km", "m",
    "in", "ft", "yd", "mi",

    // Mass
    "mg", "kg", "g",
    "lb", "oz",

    // Volume
    "ml", "mL", "cl", "dL", "L",

    // Area / volume
    "mm2", "cm2", "m2", "km2",
    "mm3", "cm3", "m3",

    // Speed
    "m/s", "km/h", "kmph", "mph", "ft/s",

    // Time
    "ns", "us", "ms", "s", "min", "h",

    // Temperature
    "°C", "°F", "K",

    // Frequency
    "Hz", "kHz", "MHz", "GHz",

    // Power
    "W", "kW", "MW", "GW",

    // Voltage
    "mV", "V", "kV",

    // Current
    "mA", "A", "kA",

    // Energy
    "J", "kJ", "MJ",
    "Wh", "kWh",

    // Data
    "B", "KB", "MB", "GB", "TB",
    "KiB", "MiB", "GiB", "TiB"
};


constexpr bool IsAsciiLetter(
    char c
) noexcept
{
    return
        (c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z');
}


std::string_view MatchUnit(
    std::string_view input,
    usize offset
) noexcept
{
    std::string_view best;

    for (const auto unit : kUnits)
    {
        if (offset + unit.size() >
            input.size())
        {
            continue;
        }

        if (input.compare(
                offset,
                unit.size(),
                unit) != 0)
        {
            continue;
        }

        const usize end =
            offset + unit.size();

        if (end < input.size() &&
            IsAsciiLetter(input[end]))
        {
            continue;
        }

        if (unit.size() > best.size())
            best = unit;
    }

    return best;
}

} // namespace

RuleMatch MeasurementRule::Match(
    std::string_view input,
    usize byte_offset
) const noexcept
{
    if (byte_offset >= input.size())
        return {};

    const NumericScan number =
        NumericScanner::ScanRaw(
            input,
            byte_offset,
            MeasurementNumericPolicy()
        );

    if (!number.matched)
        return {};

    const usize number_end =
        number.end;


    usize unit_offset =
        number_end;

    if (unit_offset < input.size() &&
        input[unit_offset] == ' ')
    {
        ++unit_offset;
    }

    const auto unit =
        MatchUnit(
            input,
            unit_offset
        );

    if (unit.empty())
        return {};

    const usize end =
        unit_offset + unit.size();

    return RuleMatch{
        RuleType::Measurement,
        byte_offset,
        end - byte_offset
    };
}

} // namespace qualix::rules
