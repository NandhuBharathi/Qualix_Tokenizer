#include "rules/currency_rule.hpp"

#include <string>

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

bool IsBoundaryContinuation(
    unsigned char c
) noexcept
{
    return
        IsAsciiAlpha(c) ||
        IsAsciiDigit(c) ||
        c == '_' ||
        c >= 0x80;
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

struct CurrencyPrefix
{
    std::string_view text;
    bool allow_space;
};

constexpr CurrencyPrefix Prefixes[] =
{
    {"$", false},
    {"€", false},
    {"£", false},
    {"¥", false},
    {"₹", false},
    {"₩", false},
    {"₽", false},
    {"₺", false},
    {"₴", false},
    {"₫", false},
    {"₱", false},
    {"₪", false},
    {"฿", false},
    {"₦", false},
    {"₡", false},
    {"₲", false},
    {"₵", false},
    {"₸", false},
    {"₼", false},
    {"₾", false},
    {"₿", false},

    {"Rs.", true},
    {"Rs", true},
    {"ரூ.", true},
    {"ரூ", true},

    {"USD", true},
    {"EUR", true},
    {"GBP", true},
    {"INR", true},
    {"JPY", true},
    {"CNY", true},
    {"RMB", true},
    {"KRW", true},
    {"AUD", true},
    {"CAD", true},
    {"CHF", true},
    {"HKD", true},
    {"SGD", true},
    {"NZD", true},
    {"SEK", true},
    {"NOK", true},
    {"DKK", true},
    {"PLN", true},
    {"CZK", true},
    {"HUF", true},
    {"RON", true},
    {"BGN", true},
    {"TRY", true},
    {"RUB", true},
    {"UAH", true},
    {"AED", true},
    {"SAR", true},
    {"QAR", true},
    {"KWD", true},
    {"BHD", true},
    {"OMR", true},
    {"JOD", true},
    {"ILS", true},
    {"EGP", true},
    {"ZAR", true},
    {"NGN", true},
    {"KES", true},
    {"GHS", true},
    {"MAD", true},
    {"MXN", true},
    {"BRL", true},
    {"ARS", true},
    {"CLP", true},
    {"COP", true},
    {"PEN", true},
    {"UYU", true},
    {"BOB", true},
    {"PYG", true},
    {"VES", true},
    {"IDR", true},
    {"MYR", true},
    {"THB", true},
    {"PHP", true},
    {"VND", true},
    {"BDT", true},
    {"PKR", true},
    {"LKR", true},
    {"NPR", true}
};

bool IsCodePrefix(
    std::string_view text
) noexcept
{
    return
        text.size() == 3 &&
        IsAsciiAlpha(
            static_cast<unsigned char>(
                text[0]
            )
        ) &&
        IsAsciiAlpha(
            static_cast<unsigned char>(
                text[1]
            )
        ) &&
        IsAsciiAlpha(
            static_cast<unsigned char>(
                text[2]
            )
        );
}

NumericScan MatchEmbeddedNumber(
    std::string_view input,
    usize number_start
) noexcept
{
    if (number_start >= input.size())
        return {};

    /*
     * NumberRule normally protects itself from
     * matching inside identifiers.
     *
     * Currency prefix parsing intentionally starts
     * immediately after a known currency marker.
     * Therefore parse the remaining substring as
     * an independent number and translate its byte
     * offsets back to the original input.
     */
    const auto remaining =
        input.substr(number_start);

    const NumericScan local =
        NumericScanner::Scan(
            remaining,
            0
        );

    if (!local.matched)
        return {};

    return NumericScan{
        number_start,
        number_start + local.ByteLength(),
        true
    };
}

} // namespace

RuleMatch CurrencyRule::Match(
    std::string_view input,
    usize byte_offset
) const noexcept
{
    if (byte_offset >= input.size())
        return {};

    /*
     * Prefix forms:
     *
     * $500
     * €500
     * ₹500
     * Rs.500
     * Rs 500
     * ரூ.500
     * USD 500
     */
    for (const auto& prefix : Prefixes)
    {
        if (!HasPrefix(
                input,
                byte_offset,
                prefix.text
            ))
        {
            continue;
        }

        if (IsCodePrefix(prefix.text))
        {
            if (byte_offset > 0)
            {
                const unsigned char previous =
                    static_cast<unsigned char>(
                        input[byte_offset - 1]
                    );

                if (IsBoundaryContinuation(previous))
                    continue;
            }
        }

        usize number_start =
            byte_offset +
            prefix.text.size();

        if (prefix.allow_space)
        {
            while (number_start <
                       input.size() &&
                   (input[number_start] == ' ' ||
                    input[number_start] == '\t'))
            {
                ++number_start;
            }
        }

        if (number_start >= input.size())
            continue;

        const NumericScan number =
            MatchEmbeddedNumber(
                input,
                number_start
            );

        if (!number.matched)
            continue;

        return RuleMatch{
            RuleType::Currency,
            byte_offset,
            number.end -
                byte_offset
        };
    }

    /*
     * Suffix ISO forms:
     *
     * 500 USD
     * 500 INR
     * 1,000.50 EUR
     */
    const NumericScan number =
        NumericScanner::Scan(
            input,
            byte_offset
        );

    if (!number.matched ||
        number.start != byte_offset)
    {
        return {};
    }

    usize code_start =
        number.end;

    while (code_start < input.size() &&
           (input[code_start] == ' ' ||
            input[code_start] == '\t'))
    {
        ++code_start;
    }

    if (code_start == number.end)
        return {};

    for (const auto& prefix : Prefixes)
    {
        if (!IsCodePrefix(prefix.text))
            continue;

        if (!HasPrefix(
                input,
                code_start,
                prefix.text
            ))
        {
            continue;
        }

        const usize match_end =
            code_start +
            prefix.text.size();

        if (match_end < input.size())
        {
            const unsigned char next =
                static_cast<unsigned char>(
                    input[match_end]
                );

            if (IsBoundaryContinuation(next))
                continue;
        }

        return RuleMatch{
            RuleType::Currency,
            byte_offset,
            match_end - byte_offset
        };
    }

    return {};
}

} // namespace qualix::rules
