#pragma once

#include <string_view>

#include "unicode/codepoint.hpp"

namespace qualix::unicode
{

enum class GraphemeBreakProperty
{
    Other,
    CR,
    LF,
    Control,
    Extend,
    ZWJ,
    RegionalIndicator,
    Prepend,
    SpacingMark,

    // Hangul syllable properties
    L,
    V,
    T,
    LV,
    LVT
};

[[nodiscard]]
GraphemeBreakProperty GetGraphemeBreakProperty(
    CodePoint codepoint
) noexcept;

[[nodiscard]]
bool IsExtendedPictographic(
    CodePoint codepoint
) noexcept;

[[nodiscard]]
constexpr std::string_view ToString(
    GraphemeBreakProperty property
) noexcept
{
    switch (property)
    {
        case GraphemeBreakProperty::Other:
            return "Other";
        case GraphemeBreakProperty::CR:
            return "CR";
        case GraphemeBreakProperty::LF:
            return "LF";
        case GraphemeBreakProperty::Control:
            return "Control";
        case GraphemeBreakProperty::Extend:
            return "Extend";
        case GraphemeBreakProperty::ZWJ:
            return "ZWJ";
        case GraphemeBreakProperty::RegionalIndicator:
            return "RegionalIndicator";
        case GraphemeBreakProperty::Prepend:
            return "Prepend";
        case GraphemeBreakProperty::SpacingMark:
            return "SpacingMark";
        case GraphemeBreakProperty::L:
            return "L";
        case GraphemeBreakProperty::V:
            return "V";
        case GraphemeBreakProperty::T:
            return "T";
        case GraphemeBreakProperty::LV:
            return "LV";
        case GraphemeBreakProperty::LVT:
            return "LVT";
    }

    return "Other";
}

} // namespace qualix::unicode
