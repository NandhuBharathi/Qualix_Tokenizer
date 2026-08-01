#include "unicode/emoji_property.hpp"

#include "unicode/generated/emoji_property_tables.hpp"
#include "unicode/grapheme_property.hpp"

namespace qualix::unicode
{

namespace
{

template <typename RangeTable>
bool InRanges(
    CodePoint codepoint,
    const RangeTable& ranges
) noexcept
{
    usize left = 0;
    usize right = ranges.size();

    while (left < right)
    {
        const usize middle =
            left + (right - left) / 2;

        const auto& range =
            ranges[middle];

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

} // namespace

bool IsEmoji(
    CodePoint codepoint
) noexcept
{
    return InRanges(
        codepoint,
        generated::EmojiRanges
    );
}

bool IsEmojiPresentation(
    CodePoint codepoint
) noexcept
{
    return InRanges(
        codepoint,
        generated::EmojiPresentationRanges
    );
}

bool IsEmojiModifier(
    CodePoint codepoint
) noexcept
{
    return InRanges(
        codepoint,
        generated::EmojiModifierRanges
    );
}

bool IsEmojiModifierBase(
    CodePoint codepoint
) noexcept
{
    return InRanges(
        codepoint,
        generated::EmojiModifierBaseRanges
    );
}

bool IsRegionalIndicator(
    CodePoint codepoint
) noexcept
{
    return
        GetGraphemeBreakProperty(codepoint) ==
        GraphemeBreakProperty::RegionalIndicator;
}

} // namespace qualix::unicode
