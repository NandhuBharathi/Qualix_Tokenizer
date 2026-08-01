#include "rules/date_rule.hpp"

namespace qualix::rules
{

namespace
{

bool IsDigit(char c) noexcept
{
    return c >= '0' && c <= '9';
}

bool IsSeparator(char c) noexcept
{
    return c == '/' ||
           c == '-' ||
           c == '.';
}

bool IsIdentifierContinuation(
    unsigned char c
) noexcept
{
    return
        (c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') ||
        c == '_' ||
        c >= 0x80;
}

struct Component
{
    usize end = 0;
    unsigned value = 0;
    unsigned digits = 0;
    bool valid = false;
};

Component ParseComponent(
    std::string_view input,
    usize offset,
    unsigned max_digits
) noexcept
{
    Component result;
    result.end = offset;

    usize i = offset;

    while (i < input.size() &&
           IsDigit(input[i]) &&
           result.digits < max_digits)
    {
        result.value =
            result.value * 10 +
            static_cast<unsigned>(
                input[i] - '0'
            );

        ++result.digits;
        ++i;
    }

    if (result.digits == 0)
        return result;

    if (i < input.size() &&
        IsDigit(input[i]))
    {
        return result;
    }

    result.end = i;
    result.valid = true;

    return result;
}

bool IsLeapYear(
    unsigned year
) noexcept
{
    if (year % 400 == 0)
        return true;

    if (year % 100 == 0)
        return false;

    return year % 4 == 0;
}

unsigned DaysInMonth(
    unsigned year,
    unsigned month
) noexcept
{
    switch (month)
    {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            return 31;

        case 4:
        case 6:
        case 9:
        case 11:
            return 30;

        case 2:
            return IsLeapYear(year)
                ? 29
                : 28;

        default:
            return 0;
    }
}

bool ValidDate(
    unsigned year,
    unsigned month,
    unsigned day
) noexcept
{
    if (year == 0 ||
        month < 1 ||
        month > 12 ||
        day < 1)
    {
        return false;
    }

    return day <=
        DaysInMonth(year, month);
}

unsigned ExpandTwoDigitYear(
    unsigned year
) noexcept
{
    return year <= 68
        ? 2000 + year
        : 1900 + year;
}

usize MatchDayFirst(
    std::string_view input,
    usize offset
) noexcept
{
    const auto day =
        ParseComponent(
            input,
            offset,
            2
        );

    if (!day.valid ||
        day.value < 1 ||
        day.value > 31 ||
        day.end >= input.size() ||
        !IsSeparator(input[day.end]))
    {
        return offset;
    }

    const char separator =
        input[day.end];

    const auto month =
        ParseComponent(
            input,
            day.end + 1,
            2
        );

    if (!month.valid ||
        month.value < 1 ||
        month.value > 12 ||
        month.end >= input.size() ||
        input[month.end] != separator)
    {
        return offset;
    }

    const auto year =
        ParseComponent(
            input,
            month.end + 1,
            4
        );

    if (!year.valid ||
        (year.digits != 2 &&
         year.digits != 4))
    {
        return offset;
    }

    const unsigned full_year =
        year.digits == 2
            ? ExpandTwoDigitYear(
                  year.value
              )
            : year.value;

    if (!ValidDate(
            full_year,
            month.value,
            day.value))
    {
        return offset;
    }

    return year.end;
}

usize MatchYearFirst(
    std::string_view input,
    usize offset
) noexcept
{
    const auto year =
        ParseComponent(
            input,
            offset,
            4
        );

    if (!year.valid ||
        year.digits != 4 ||
        year.value == 0 ||
        year.end >= input.size() ||
        !IsSeparator(input[year.end]))
    {
        return offset;
    }

    const char separator =
        input[year.end];

    const auto month =
        ParseComponent(
            input,
            year.end + 1,
            2
        );

    if (!month.valid ||
        month.value < 1 ||
        month.value > 12 ||
        month.end >= input.size() ||
        input[month.end] != separator)
    {
        return offset;
    }

    const auto day =
        ParseComponent(
            input,
            month.end + 1,
            2
        );

    if (!day.valid ||
        !ValidDate(
            year.value,
            month.value,
            day.value))
    {
        return offset;
    }

    return day.end;
}

bool ValidBoundary(
    std::string_view input,
    usize offset,
    usize end
) noexcept
{
    if (offset > 0)
    {
        const unsigned char previous =
            static_cast<unsigned char>(
                input[offset - 1]
            );

        if (IsIdentifierContinuation(
                previous))
        {
            return false;
        }
    }

    if (end < input.size())
    {
        const unsigned char next =
            static_cast<unsigned char>(
                input[end]
            );

        if (IsIdentifierContinuation(
                next))
        {
            return false;
        }
    }

    return true;
}

} // namespace

RuleMatch DateRule::Match(
    std::string_view input,
    usize byte_offset
) const noexcept
{
    if (byte_offset >= input.size() ||
        !IsDigit(input[byte_offset]))
    {
        return {};
    }

    usize end =
        MatchYearFirst(
            input,
            byte_offset
        );

    if (end == byte_offset)
    {
        end =
            MatchDayFirst(
                input,
                byte_offset
            );
    }

    if (end == byte_offset ||
        !ValidBoundary(
            input,
            byte_offset,
            end))
    {
        return {};
    }

    return RuleMatch{
        RuleType::Date,
        byte_offset,
        end - byte_offset
    };
}

} // namespace qualix::rules
