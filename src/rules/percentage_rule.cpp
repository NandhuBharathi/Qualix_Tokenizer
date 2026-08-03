#include "rules/percentage_rule.hpp"

namespace qualix::rules
{

namespace
{

bool IsPerMilleAt(
    std::string_view input,
    usize offset
) noexcept
{
    return
        offset + 2 < input.size() &&
        static_cast<unsigned char>(input[offset]) == 0xE2 &&
        static_cast<unsigned char>(input[offset + 1]) == 0x80 &&
        static_cast<unsigned char>(input[offset + 2]) == 0xB0;
}

bool IsInvalidFollowing(
    unsigned char c
) noexcept
{
    const bool ascii_alpha =
        (c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z');

    const bool ascii_digit =
        c >= '0' && c <= '9';

    return
        ascii_alpha ||
        ascii_digit ||
        c == '_' ||
        c >= 0x80;
}

} // namespace

RuleMatch PercentageRule::Match(
    std::string_view input,
    usize byte_offset
) const noexcept
{
    if (byte_offset >= input.size())
        return {};

    usize suffix_start =
        std::string_view::npos;

    usize suffix_length = 0;

    for (usize i = byte_offset;
         i < input.size();
         ++i)
    {
        if (input[i] == '%')
        {
            suffix_start = i;
            suffix_length = 1;
            break;
        }

        if (IsPerMilleAt(input, i))
        {
            suffix_start = i;
            suffix_length = 3;
            break;
        }
    }

    if (suffix_start ==
        std::string_view::npos)
    {
        return {};
    }

    if (suffix_start == byte_offset)
        return {};

    const auto number_view =
        input.substr(
            byte_offset,
            suffix_start - byte_offset
        );

    const NumericScan number =
        NumericScanner::Scan(
            number_view,
            0
        );

    if (!number.matched)
        return {};

    if (number.ByteLength() !=
        number_view.size())
    {
        return {};
    }

    const usize match_end =
        suffix_start + suffix_length;

    if (match_end < input.size())
    {
        const unsigned char next =
            static_cast<unsigned char>(
                input[match_end]
            );

        if (IsInvalidFollowing(next))
            return {};
    }

    return RuleMatch{
        RuleType::Percentage,
        byte_offset,
        match_end - byte_offset
    };
}

} // namespace qualix::rules
