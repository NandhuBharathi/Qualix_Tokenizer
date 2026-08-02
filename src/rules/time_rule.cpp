#include "rules/time_rule.hpp"

#include <cctype>

namespace qualix::rules
{
namespace
{

[[nodiscard]]
bool IsDigit(char c) noexcept
{
    return c >= '0' && c <= '9';
}

[[nodiscard]]
bool IsAlpha(char c) noexcept
{
    return
        (c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z');
}

[[nodiscard]]
char Lower(char c) noexcept
{
    if (c >= 'A' && c <= 'Z')
    {
        return static_cast<char>(
            c - 'A' + 'a'
        );
    }

    return c;
}

[[nodiscard]]
bool IsAmPm(
    std::string_view input,
    usize pos
) noexcept
{
    if (pos + 2 > input.size())
    {
        return false;
    }

    const char a = Lower(input[pos]);
    const char b = Lower(input[pos + 1]);

    return
        (a == 'a' || a == 'p') &&
        b == 'm';
}

[[nodiscard]]
bool ParseTwoDigits(
    std::string_view input,
    usize pos,
    int& value
) noexcept
{
    if (pos + 2 > input.size())
    {
        return false;
    }

    if (!IsDigit(input[pos]) ||
        !IsDigit(input[pos + 1]))
    {
        return false;
    }

    value =
        (input[pos] - '0') * 10 +
        (input[pos + 1] - '0');

    return true;
}

} // namespace

RuleMatch TimeRule::Match(
    std::string_view input,
    usize byte_offset
) const noexcept
{
    if (byte_offset >= input.size())
    {
        return {};
    }

    usize pos = byte_offset;

    if (!IsDigit(input[pos]))
    {
        return {};
    }

    int hour = 0;
    usize hour_digits = 0;

    while (
        pos < input.size() &&
        IsDigit(input[pos]) &&
        hour_digits < 2
    )
    {
        hour =
            hour * 10 +
            (input[pos] - '0');

        ++pos;
        ++hour_digits;
    }

    if (
        pos < input.size() &&
        IsDigit(input[pos])
    )
    {
        return {};
    }

    bool has_colon = false;

    if (
        pos < input.size() &&
        input[pos] == ':'
    )
    {
        has_colon = true;
        ++pos;

        int minute = 0;

        if (!ParseTwoDigits(
                input,
                pos,
                minute))
        {
            return {};
        }

        if (minute > 59)
        {
            return {};
        }

        pos += 2;

        if (
            pos < input.size() &&
            input[pos] == ':'
        )
        {
            ++pos;

            int second = 0;

            if (!ParseTwoDigits(
                    input,
                    pos,
                    second))
            {
                return {};
            }

            if (second > 59)
            {
                return {};
            }

            pos += 2;
        }
    }

    usize marker_pos = pos;

    if (
        marker_pos < input.size() &&
        input[marker_pos] == ' '
    )
    {
        ++marker_pos;
    }

    const bool has_ampm =
        IsAmPm(input, marker_pos);

    if (!has_colon && !has_ampm)
    {
        return {};
    }

    if (has_ampm)
    {
        if (hour < 1 || hour > 12)
        {
            return {};
        }

        pos = marker_pos + 2;
    }
    else
    {
        if (hour > 23)
        {
            return {};
        }
    }

    if (
        pos < input.size() &&
        IsAlpha(input[pos])
    )
    {
        return {};
    }

    return RuleMatch{
        RuleType::Time,
        byte_offset,
        pos - byte_offset
    };
}

} // namespace qualix::rules
