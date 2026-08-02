#pragma once

#include <string_view>

namespace qualix::pretokenizer
{

enum class SpanType
{
    Unknown = 0,

    Word,
    Number,
    Whitespace,
    Punctuation,
    Symbol,
    Emoji,

    Url,
    Email,
    Date,
    Time,
    Phone,
    Currency,
    Percentage,
    Measurement,
    Math,
    Code,

    Other
};

constexpr std::string_view ToString(
    SpanType type
) noexcept
{
    switch (type)
    {
        case SpanType::Unknown:
            return "Unknown";

        case SpanType::Word:
            return "Word";

        case SpanType::Number:
            return "Number";

        case SpanType::Whitespace:
            return "Whitespace";

        case SpanType::Punctuation:
            return "Punctuation";

        case SpanType::Symbol:
            return "Symbol";

        case SpanType::Emoji:
            return "Emoji";

        case SpanType::Url:
            return "Url";

        case SpanType::Email:
            return "Email";

        case SpanType::Date:
            return "Date";

        case SpanType::Time:
            return "Time";

        case SpanType::Phone:
            return "Phone";

        case SpanType::Currency:
            return "Currency";

        case SpanType::Percentage:
            return "Percentage";

        case SpanType::Measurement:
            return "Measurement";

        case SpanType::Math:
            return "Math";

        case SpanType::Code:
            return "Code";

        case SpanType::Other:
            return "Other";
    }

    return "Unknown";
}

} // namespace qualix::pretokenizer
