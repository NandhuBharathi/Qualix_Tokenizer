#include "rules/email_rule.hpp"

namespace qualix::rules
{

namespace
{

bool IsAsciiAlpha(
    unsigned char c
) noexcept
{
    return
        (c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z');
}

bool IsAsciiDigit(
    unsigned char c
) noexcept
{
    return c >= '0' && c <= '9';
}

bool IsLocalAtom(
    unsigned char c
) noexcept
{
    return
        IsAsciiAlpha(c) ||
        IsAsciiDigit(c) ||
        c == '!' ||
        c == '#' ||
        c == '$' ||
        c == '%' ||
        c == '&' ||
        c == '\'' ||
        c == '*' ||
        c == '+' ||
        c == '-' ||
        c == '/' ||
        c == '=' ||
        c == '?' ||
        c == '^' ||
        c == '_' ||
        c == '`' ||
        c == '{' ||
        c == '|' ||
        c == '}' ||
        c == '~' ||
        c >= 0x80;
}

bool IsDomainAtom(
    unsigned char c
) noexcept
{
    return
        IsAsciiAlpha(c) ||
        IsAsciiDigit(c) ||
        c == '-' ||
        c >= 0x80;
}

bool IsBoundaryChar(
    unsigned char c
) noexcept
{
    return
        IsLocalAtom(c) ||
        c == '.' ||
        c == '@';
}

bool ParseLocalPart(
    std::string_view input,
    usize start,
    usize& at
) noexcept
{
    usize i = start;

    bool atom_has_char = false;

    while (i < input.size())
    {
        const unsigned char c =
            static_cast<unsigned char>(
                input[i]
            );

        if (c == '@')
        {
            if (!atom_has_char)
                return false;

            at = i;
            return i > start;
        }

        if (c == '.')
        {
            if (!atom_has_char)
                return false;

            atom_has_char = false;
            ++i;
            continue;
        }

        if (!IsLocalAtom(c))
            return false;

        atom_has_char = true;
        ++i;
    }

    return false;
}

bool ParseDomain(
    std::string_view input,
    usize start,
    usize& end
) noexcept
{
    usize i = start;

    bool label_has_char = false;
    bool has_dot = false;

    while (i < input.size())
    {
        const unsigned char c =
            static_cast<unsigned char>(
                input[i]
            );

        if (c == '.')
        {
            if (!label_has_char)
                break;

            if (input[i - 1] == '-')
                break;

            if (i + 1 >= input.size())
                break;

            const unsigned char next =
                static_cast<unsigned char>(
                    input[i + 1]
                );

            if (!IsDomainAtom(next) ||
                next == '-')
            {
                break;
            }

            has_dot = true;
            label_has_char = false;
            ++i;
            continue;
        }

        if (!IsDomainAtom(c))
            break;

        if (!label_has_char &&
            c == '-')
        {
            break;
        }

        label_has_char = true;
        ++i;
    }

    if (i <= start)
        return false;

    if (!label_has_char)
        return false;

    if (input[i - 1] == '-')
        return false;

    if (!has_dot)
        return false;

    end = i;
    return true;
}

} // namespace

RuleMatch EmailRule::Match(
    std::string_view input,
    usize byte_offset
) const noexcept
{
    if (byte_offset >= input.size())
        return {};

    if (byte_offset > 0)
    {
        const unsigned char previous =
            static_cast<unsigned char>(
                input[byte_offset - 1]
            );

        if (IsBoundaryChar(previous))
            return {};
    }

    usize at = 0;

    if (!ParseLocalPart(
            input,
            byte_offset,
            at
        ))
    {
        return {};
    }

    if (at + 1 >= input.size())
        return {};

    usize end = 0;

    if (!ParseDomain(
            input,
            at + 1,
            end
        ))
    {
        return {};
    }

    if (end < input.size())
    {
        const unsigned char next =
            static_cast<unsigned char>(
                input[end]
            );

        if (next == '@' ||
            IsLocalAtom(next))
        {
            return {};
        }

        if (next == '.' &&
            end + 1 < input.size())
        {
            const unsigned char after_dot =
                static_cast<unsigned char>(
                    input[end + 1]
                );

            if (IsDomainAtom(after_dot))
                return {};
        }
    }

    return RuleMatch{
        RuleType::Email,
        byte_offset,
        end - byte_offset
    };
}

} // namespace qualix::rules
