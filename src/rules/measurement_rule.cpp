#include "rules/measurement_rule.hpp"

#include <array>
#include <string_view>

namespace qualix::rules
{

namespace
{

constexpr std::array<std::string_view, 34> kUnits = {
    "mm", "cm", "km", "m",
    "mg", "kg", "g",
    "ml", "mL", "L",
    "in", "ft", "yd", "mi",
    "lb", "oz",
    "°C", "°F",
    "Hz", "kHz", "MHz", "GHz",
    "W", "kW", "MW",
    "V", "mV", "kV",
    "B", "KB", "MB", "GB", "TB",
    "ms"
};

constexpr bool IsDigit(
    char c
) noexcept
{
    return c >= '0' && c <= '9';
}

constexpr bool IsAsciiLetter(
    char c
) noexcept
{
    return
        (c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z');
}

usize MatchNumberPrefix(
    std::string_view input,
    usize offset
) noexcept
{
    usize i = offset;

    if (i < input.size() &&
        (input[i] == '+' ||
         input[i] == '-'))
    {
        ++i;
    }

    const usize integer_start = i;

    while (i < input.size() &&
           IsDigit(input[i]))
    {
        ++i;
    }

    if (i == integer_start)
        return offset;

    if (i < input.size() &&
        input[i] == '.')
    {
        const usize dot = i;
        ++i;

        const usize fraction_start = i;

        while (i < input.size() &&
               IsDigit(input[i]))
        {
            ++i;
        }

        if (i == fraction_start)
            i = dot;
    }

    if (i < input.size() &&
        (input[i] == 'e' ||
         input[i] == 'E'))
    {
        const usize exponent = i;
        ++i;

        if (i < input.size() &&
            (input[i] == '+' ||
             input[i] == '-'))
        {
            ++i;
        }

        const usize exponent_start = i;

        while (i < input.size() &&
               IsDigit(input[i]))
        {
            ++i;
        }

        if (i == exponent_start)
            i = exponent;
    }

    return i;
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

    const usize number_end =
        MatchNumberPrefix(
            input,
            byte_offset
        );

    if (number_end == byte_offset)
        return {};

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
