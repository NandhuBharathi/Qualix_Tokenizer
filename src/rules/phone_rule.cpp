#include "rules/phone_rule.hpp"

namespace qualix::rules
{

namespace
{

bool IsDigit(char c) noexcept
{
    return c >= '0' && c <= '9';
}

bool IsAsciiAlpha(char c) noexcept
{
    return
        (c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z');
}

bool IsSeparator(char c) noexcept
{
    return
        c == ' ' ||
        c == '-';
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

bool LooksLikeIsoDate(
    std::string_view input,
    usize offset,
    usize end
) noexcept
{
    if (end - offset != 10)
        return false;

    return
        IsDigit(input[offset]) &&
        IsDigit(input[offset + 1]) &&
        IsDigit(input[offset + 2]) &&
        IsDigit(input[offset + 3]) &&
        input[offset + 4] == '-' &&
        IsDigit(input[offset + 5]) &&
        IsDigit(input[offset + 6]) &&
        input[offset + 7] == '-' &&
        IsDigit(input[offset + 8]) &&
        IsDigit(input[offset + 9]);
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

        if (IsIdentifierContinuation(previous))
            return false;
    }

    if (end < input.size())
    {
        const unsigned char next =
            static_cast<unsigned char>(
                input[end]
            );

        if (IsIdentifierContinuation(next))
            return false;
    }

    return true;
}

} // namespace

RuleMatch PhoneRule::Match(
    std::string_view input,
    usize byte_offset
) const noexcept
{
    if (byte_offset >= input.size())
        return {};

    usize pos = byte_offset;
    usize digit_count = 0;

    // Optional international prefix.
    if (input[pos] == '+')
    {
        ++pos;

        if (pos >= input.size() ||
            !IsDigit(input[pos]))
        {
            return {};
        }
    }

    // Optional parenthesized area code.
    if (pos < input.size() &&
        input[pos] == '(')
    {
        ++pos;

        usize area_digits = 0;

        while (pos < input.size() &&
               IsDigit(input[pos]) &&
               area_digits < 4)
        {
            ++pos;
            ++area_digits;
            ++digit_count;
        }

        if (area_digits < 2 ||
            pos >= input.size() ||
            input[pos] != ')')
        {
            return {};
        }

        ++pos;

        if (pos < input.size() &&
            IsSeparator(input[pos]))
        {
            ++pos;
        }
    }

    while (pos < input.size())
    {
        const char c = input[pos];

        if (IsDigit(c))
        {
            ++digit_count;

            // E.164 maximum.
            if (digit_count > 15)
                return {};

            ++pos;
            continue;
        }

        if (IsSeparator(c))
        {
            // Separator must be between digits.
            if (pos == byte_offset ||
                pos + 1 >= input.size() ||
                !IsDigit(input[pos + 1]))
            {
                break;
            }

            ++pos;
            continue;
        }

        break;
    }

    // Avoid ordinary short numbers being classified
    // as phone numbers.
    if (digit_count < 7 ||
        digit_count > 15)
    {
        return {};
    }

    if (pos <= byte_offset ||
        !ValidBoundary(
            input,
            byte_offset,
            pos))
    {
        return {};
    }

    if (LooksLikeIsoDate(
            input,
            byte_offset,
            pos))
    {
        return {};
    }

    // Reject alphabetic continuation explicitly.
    if (pos < input.size() &&
        IsAsciiAlpha(input[pos]))
    {
        return {};
    }

    return RuleMatch{
        RuleType::Phone,
        byte_offset,
        pos - byte_offset
    };
}

} // namespace qualix::rules
