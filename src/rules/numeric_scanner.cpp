#include "rules/numeric_scanner.hpp"

namespace qualix::rules
{

namespace
{

bool IsDigit(char c) noexcept
{
    return c >= '0' && c <= '9';
}

bool IsHexDigit(char c) noexcept
{
    return
        IsDigit(c) ||
        (c >= 'a' && c <= 'f') ||
        (c >= 'A' && c <= 'F');
}

bool IsBinaryDigit(char c) noexcept
{
    return c == '0' || c == '1';
}

bool IsOctalDigit(char c) noexcept
{
    return c >= '0' && c <= '7';
}

bool IsAsciiLetter(char c) noexcept
{
    return
        (c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z');
}

bool IsIdentifierContinuation(
    unsigned char c
) noexcept
{
    return
        IsAsciiLetter(
            static_cast<char>(c)
        ) ||
        IsDigit(
            static_cast<char>(c)
        ) ||
        c == '_' ||
        c >= 0x80;
}

template <typename Predicate>
usize ParseBasedDigits(
    std::string_view input,
    usize offset,
    Predicate predicate
) noexcept
{
    usize i = offset;
    bool has_digit = false;
    bool previous_separator = false;

    while (i < input.size())
    {
        const char c = input[i];

        if (predicate(c))
        {
            has_digit = true;
            previous_separator = false;
            ++i;
            continue;
        }

        if (c == '_')
        {
            if (!has_digit ||
                previous_separator ||
                i + 1 >= input.size() ||
                !predicate(input[i + 1]))
            {
                break;
            }

            previous_separator = true;
            ++i;
            continue;
        }

        break;
    }

    if (!has_digit ||
        previous_separator)
    {
        return offset;
    }

    return i;
}

usize ParseBaseNumber(
    std::string_view input,
    usize offset
) noexcept
{
    if (offset + 2 > input.size() ||
        input[offset] != '0')
    {
        return offset;
    }

    const char prefix =
        input[offset + 1];

    usize end = offset;

    if (prefix == 'x' ||
        prefix == 'X')
    {
        end = ParseBasedDigits(
            input,
            offset + 2,
            IsHexDigit
        );
    }
    else if (prefix == 'b' ||
             prefix == 'B')
    {
        end = ParseBasedDigits(
            input,
            offset + 2,
            IsBinaryDigit
        );
    }
    else if (prefix == 'o' ||
             prefix == 'O')
    {
        end = ParseBasedDigits(
            input,
            offset + 2,
            IsOctalDigit
        );
    }
    else
    {
        return offset;
    }

    if (end == offset + 2)
        return offset;

    if (end < input.size())
    {
        const unsigned char next =
            static_cast<unsigned char>(
                input[end]
            );

        if (IsIdentifierContinuation(next))
            return offset;
    }

    return end;
}

usize ParseIntegerPart(
    std::string_view input,
    usize offset,
    const NumericScanPolicy& policy
) noexcept
{
    usize i = offset;

    if (i >= input.size() ||
        !IsDigit(input[i]))
    {
        return offset;
    }

    const usize first_start = i;

    while (i < input.size() &&
           IsDigit(input[i]))
    {
        ++i;
    }

    const usize first_digits =
        i - first_start;

    if (policy.allow_grouping &&
        i < input.size() &&
        input[i] == ',')
    {
        if (first_digits < 1 ||
            first_digits > 3)
        {
            return i;
        }

        const usize comma_start = i;

        usize scan = i;
        usize group_count = 0;
        usize group_sizes[32]{};

        while (scan < input.size() &&
               input[scan] == ',')
        {
            if (group_count >= 32)
                return comma_start;

            ++scan;

            const usize group_start =
                scan;

            while (scan < input.size() &&
                   IsDigit(input[scan]))
            {
                ++scan;
            }

            const usize digits =
                scan - group_start;

            if (digits == 0)
                return comma_start;

            group_sizes[group_count++] =
                digits;

            if (scan >= input.size() ||
                input[scan] != ',')
            {
                break;
            }
        }

        bool international = true;

        for (usize g = 0;
             g < group_count;
             ++g)
        {
            if (group_sizes[g] != 3)
            {
                international = false;
                break;
            }
        }

        bool indian =
            group_count >= 2;

        if (indian)
        {
            for (usize g = 0;
                 g + 1 < group_count;
                 ++g)
            {
                if (group_sizes[g] != 2)
                {
                    indian = false;
                    break;
                }
            }

            if (group_sizes[group_count - 1] != 3)
                indian = false;
        }

        if (!international &&
            !indian)
        {
            return comma_start;
        }

        i = scan;
        return i;
    }

    while (policy.allow_underscore &&
           i < input.size() &&
           input[i] == '_')
    {
        const usize separator = i;

        ++i;

        usize digits = 0;

        while (i < input.size() &&
               IsDigit(input[i]))
        {
            ++i;
            ++digits;
        }

        if (digits == 0)
            return separator;
    }

    return i;
}

usize ParseDecimal(
    std::string_view input,
    usize offset,
    const NumericScanPolicy& policy
) noexcept
{
    usize i = offset;
    bool has_integer = false;
    bool has_fraction = false;

    if (i < input.size() &&
        IsDigit(input[i]))
    {
        const usize integer_end =
            ParseIntegerPart(
                input,
                i,
                policy
            );

        if (integer_end == i)
            return offset;

        i = integer_end;
        has_integer = true;
    }

    /*
     * Consume '.' only when digits follow it.
     *
     * 2.      -> Number "2" + punctuation "."
     * 3.14    -> Number "3.14"
     * .5      -> Number ".5"
     */
    if (i < input.size() &&
        input[i] == '.' &&
        (has_integer ||
         policy.allow_leading_dot))
    {
        ++i;

        const usize fraction_start = i;

        while (i < input.size() &&
               IsDigit(input[i]))
        {
            ++i;
        }

        has_fraction =
            i > fraction_start;

        if (!has_fraction &&
            has_integer &&
            !policy.allow_trailing_dot)
        {
            --i;
        }

        if (!has_integer &&
            !has_fraction)
        {
            return offset;
        }
    }

    if (!has_integer &&
        !has_fraction)
    {
        return offset;
    }

    if (policy.allow_exponent &&
        i < input.size() &&
        (input[i] == 'e' ||
         input[i] == 'E'))
    {
        const usize exponent_mark = i;
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
            i = exponent_mark;
    }

    return i;
}

bool ValidStartBoundary(
    std::string_view input,
    usize offset
) noexcept
{
    if (offset == 0)
        return true;

    const unsigned char previous =
        static_cast<unsigned char>(
            input[offset - 1]
        );

    return
        !IsIdentifierContinuation(
            previous
        );
}

bool ValidEndBoundary(
    std::string_view input,
    usize end
) noexcept
{
    if (end >= input.size())
        return true;

    const unsigned char next =
        static_cast<unsigned char>(
            input[end]
        );

    return
        !IsIdentifierContinuation(
            next
        );
}

} // namespace


NumericScan NumericScanner::ScanRaw(
    std::string_view input,
    usize offset
) noexcept
{
    static constexpr NumericScanPolicy policy{};

    return ScanRaw(
        input,
        offset,
        policy
    );
}

NumericScan NumericScanner::ScanRaw(
    std::string_view input,
    usize offset,
    const NumericScanPolicy& policy
) noexcept
{
    if (offset >= input.size())
        return {};

    const usize start=offset;
    usize i=offset;

    if (policy.allow_sign &&
        (input[i]=='+' ||
         input[i]=='-'))
    {
        ++i;

        if (i>=input.size())
            return {};
    }

    const usize base_end=
        policy.allow_based_numbers
            ? ParseBaseNumber(
                  input,
                  i
              )
            : i;

    usize end=i;

    if (base_end!=i)
    {
        end=base_end;
    }
    else
    {
        end=ParseDecimal(
            input,
            i,
            policy
        );

        if (end==i)
            return {};
    }

    return NumericScan{
        start,
        end,
        true
    };
}

NumericScan NumericScanner::Scan(
    std::string_view input,
    usize offset
) noexcept
{
    if (offset >= input.size())
        return {};

    if (!ValidStartBoundary(
            input,
            offset))
    {
        return {};
    }

    const NumericScan scan=
        ScanRaw(
            input,
            offset
        );

    if (!scan.matched)
        return {};

    if (!ValidEndBoundary(
            input,
            scan.end))
    {
        return {};
    }

    return scan;
}

} // namespace qualix::rules
