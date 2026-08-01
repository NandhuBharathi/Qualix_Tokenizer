#pragma once

#include <string_view>

namespace qualix::pretokenizer
{

enum class GraphemeClass
{
    Other = 0,
    Letter,
    Number,
    Whitespace,
    Punctuation,
    Symbol,
    Emoji,
    Control
};

constexpr std::string_view ToString(
    GraphemeClass value
) noexcept
{
    switch (value)
    {
        case GraphemeClass::Other:
            return "Other";

        case GraphemeClass::Letter:
            return "Letter";

        case GraphemeClass::Number:
            return "Number";

        case GraphemeClass::Whitespace:
            return "Whitespace";

        case GraphemeClass::Punctuation:
            return "Punctuation";

        case GraphemeClass::Symbol:
            return "Symbol";

        case GraphemeClass::Emoji:
            return "Emoji";

        case GraphemeClass::Control:
            return "Control";
    }

    return "Other";
}

} // namespace qualix::pretokenizer
