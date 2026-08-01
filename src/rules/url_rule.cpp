#include "rules/url_rule.hpp"

namespace qualix::rules
{

namespace
{

bool IsAsciiAlpha(
    char c
) noexcept
{
    return
        (c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z');
}

bool IsAsciiDigit(
    char c
) noexcept
{
    return c >= '0' && c <= '9';
}

bool IsDomainChar(
    unsigned char c
) noexcept
{
    return
        IsAsciiAlpha(static_cast<char>(c)) ||
        IsAsciiDigit(static_cast<char>(c)) ||
        c == '-' ||
        c >= 0x80;
}

bool IsUrlStop(
    unsigned char c
) noexcept
{
    return
        c <= 0x20 ||
        c == '"' ||
        c == '\'' ||
        c == '<' ||
        c == '>' ||
        c == '`';
}

bool IsTrailingPunctuation(
    unsigned char c
) noexcept
{
    return
        c == '.' ||
        c == ',' ||
        c == '!' ||
        c == '?' ||
        c == ';' ||
        c == ':';
}

bool HasPrefix(
    std::string_view input,
    usize offset,
    std::string_view prefix
) noexcept
{
    if (offset > input.size())
        return false;

    if (prefix.size() >
        input.size() - offset)
    {
        return false;
    }

    return input.substr(
        offset,
        prefix.size()
    ) == prefix;
}

usize ScanUrlEnd(
    std::string_view input,
    usize start
) noexcept
{
    usize end = start;

    int parentheses = 0;
    int brackets = 0;
    int braces = 0;

    while (end < input.size())
    {
        const unsigned char c =
            static_cast<unsigned char>(
                input[end]
            );

        if (IsUrlStop(c))
            break;

        if (c == '(')
        {
            ++parentheses;
        }
        else if (c == ')')
        {
            if (parentheses == 0)
                break;

            --parentheses;
        }
        else if (c == '[')
        {
            ++brackets;
        }
        else if (c == ']')
        {
            if (brackets == 0)
                break;

            --brackets;
        }
        else if (c == '{')
        {
            ++braces;
        }
        else if (c == '}')
        {
            if (braces == 0)
                break;

            --braces;
        }

        ++end;
    }

    while (end > start)
    {
        const unsigned char c =
            static_cast<unsigned char>(
                input[end - 1]
            );

        if (!IsTrailingPunctuation(c))
            break;

        --end;
    }

    return end;
}

bool HasValidDomain(
    std::string_view text
) noexcept
{
    if (text.empty())
        return false;

    usize host_end = 0;

    while (host_end < text.size())
    {
        const unsigned char c =
            static_cast<unsigned char>(
                text[host_end]
            );

        if (c == '/' ||
            c == '?' ||
            c == '#' ||
            c == ':')
        {
            break;
        }

        ++host_end;
    }

    if (host_end == 0)
        return false;

    const auto host =
        text.substr(0, host_end);

    if (host.empty())
        return false;

    if (IsAsciiDigit(
            static_cast<char>(
                static_cast<unsigned char>(
                    host.front()
                )
            )
        ))
    {
        return false;
    }

    bool has_dot = false;
    bool label_has_char = false;
    bool final_label_has_non_digit = false;

    for (usize i = 0;
         i < host.size();
         ++i)
    {
        const unsigned char c =
            static_cast<unsigned char>(
                host[i]
            );

        if (c == '.')
        {
            if (!label_has_char)
                return false;

            if (i > 0 &&
                host[i - 1] == '-')
            {
                return false;
            }

            has_dot = true;
            label_has_char = false;
            final_label_has_non_digit = false;
            continue;
        }

        if (!IsDomainChar(c))
            return false;

        if (!label_has_char &&
            c == '-')
        {
            return false;
        }

        if (!IsAsciiDigit(
                static_cast<char>(c)
            ))
        {
            final_label_has_non_digit = true;
        }

        label_has_char = true;
    }

    if (!label_has_char)
        return false;

    if (host.back() == '-')
        return false;

    if (!final_label_has_non_digit)
        return false;

    return has_dot;
}

} // namespace

RuleMatch UrlRule::Match(
    std::string_view input,
    usize byte_offset
) const noexcept
{
    if (byte_offset >= input.size())
        return {};

    usize content_start =
        byte_offset;

    bool explicit_url = false;

    if (HasPrefix(
            input,
            byte_offset,
            "https://"
        ))
    {
        content_start =
            byte_offset + 8;

        explicit_url = true;
    }
    else if (HasPrefix(
                 input,
                 byte_offset,
                 "http://"
             ))
    {
        content_start =
            byte_offset + 7;

        explicit_url = true;
    }
    else if (HasPrefix(
                 input,
                 byte_offset,
                 "www."
             ))
    {
        content_start =
            byte_offset;

        explicit_url = true;
    }

    const usize end =
        ScanUrlEnd(
            input,
            content_start
        );

    if (end <= content_start)
        return {};

    const auto candidate =
        input.substr(
            content_start,
            end - content_start
        );

    if (!HasValidDomain(candidate))
        return {};

    if (!explicit_url)
    {
        if (byte_offset > 0)
        {
            const unsigned char previous =
                static_cast<unsigned char>(
                    input[byte_offset - 1]
                );

            if (IsDomainChar(previous) ||
                previous == '.' ||
                previous == '@')
            {
                return {};
            }
        }
    }

    return RuleMatch{
        RuleType::Url,
        byte_offset,
        end - byte_offset
    };
}

} // namespace qualix::rules
