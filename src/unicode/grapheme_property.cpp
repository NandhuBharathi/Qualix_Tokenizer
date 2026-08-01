#include "unicode/grapheme_property.hpp"
#include "unicode/generated/grapheme_tables.hpp"

namespace qualix::unicode
{

GraphemeBreakProperty GetGraphemeBreakProperty(
    CodePoint codepoint
) noexcept
{
    const auto& ranges = generated::GraphemePropertyRanges;

    usize left = 0;
    usize right = ranges.size();

    while (left < right)
    {
        const usize middle = left + (right - left) / 2;
        const auto& range = ranges[middle];

        if (codepoint < range.first)
        {
            right = middle;
        }
        else if (codepoint > range.last)
        {
            left = middle + 1;
        }
        else
        {
            return range.property;
        }
    }

    return GraphemeBreakProperty::Other;
}

bool IsExtendedPictographic(
    CodePoint codepoint
) noexcept
{
    const auto& ranges = generated::ExtendedPictographicRanges;

    usize left = 0;
    usize right = ranges.size();

    while (left < right)
    {
        const usize middle = left + (right - left) / 2;
        const auto& range = ranges[middle];

        if (codepoint < range.first)
        {
            right = middle;
        }
        else if (codepoint > range.last)
        {
            left = middle + 1;
        }
        else
        {
            return true;
        }
    }

    return false;
}

} // namespace qualix::unicode
