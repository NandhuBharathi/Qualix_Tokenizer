#include "pretokenizer/grapheme_classifier.hpp"

#include "unicode/emoji_property.hpp"
#include "unicode/general_category.hpp"
#include "unicode/grapheme_property.hpp"
#include "unicode/utf8.hpp"

namespace qualix::pretokenizer
{

namespace
{

bool IsLetter(
    unicode::GeneralCategory category
) noexcept
{
    using unicode::GeneralCategory;

    return
        category == GeneralCategory::Lu ||
        category == GeneralCategory::Ll ||
        category == GeneralCategory::Lt ||
        category == GeneralCategory::Lm ||
        category == GeneralCategory::Lo;
}

bool IsMark(
    unicode::GeneralCategory category
) noexcept
{
    using unicode::GeneralCategory;

    return
        category == GeneralCategory::Mn ||
        category == GeneralCategory::Mc ||
        category == GeneralCategory::Me;
}

bool IsNumber(
    unicode::GeneralCategory category
) noexcept
{
    using unicode::GeneralCategory;

    return
        category == GeneralCategory::Nd ||
        category == GeneralCategory::Nl ||
        category == GeneralCategory::No;
}

bool IsPunctuation(
    unicode::GeneralCategory category
) noexcept
{
    using unicode::GeneralCategory;

    return
        category == GeneralCategory::Pc ||
        category == GeneralCategory::Pd ||
        category == GeneralCategory::Ps ||
        category == GeneralCategory::Pe ||
        category == GeneralCategory::Pi ||
        category == GeneralCategory::Pf ||
        category == GeneralCategory::Po;
}

bool IsSymbol(
    unicode::GeneralCategory category
) noexcept
{
    using unicode::GeneralCategory;

    return
        category == GeneralCategory::Sm ||
        category == GeneralCategory::Sc ||
        category == GeneralCategory::Sk ||
        category == GeneralCategory::So;
}

bool IsWhitespace(
    unicode::GeneralCategory category,
    unicode::CodePoint cp
) noexcept
{
    using unicode::GeneralCategory;

    if (category == GeneralCategory::Zs ||
        category == GeneralCategory::Zl ||
        category == GeneralCategory::Zp)
    {
        return true;
    }

    return
        cp == 0x0009 ||
        cp == 0x000A ||
        cp == 0x000B ||
        cp == 0x000C ||
        cp == 0x000D;
}

bool IsControl(
    unicode::GeneralCategory category
) noexcept
{
    using unicode::GeneralCategory;

    return
        category == GeneralCategory::Cc ||
        category == GeneralCategory::Cf ||
        category == GeneralCategory::Cs;
}

bool IsEmojiGrapheme(
    std::string_view grapheme
) noexcept
{
    usize offset = 0;

    usize regional_indicators = 0;

    bool has_default_emoji = false;
    bool has_extended_pictographic = false;
    bool has_vs16 = false;
    bool has_zwj = false;
    bool has_modifier = false;

    while (offset < grapheme.size())
    {
        auto decoded =
            unicode::Utf8::Decode(
                grapheme.substr(offset)
            );

        if (decoded.Failed())
            return false;

        const auto value = decoded.Value();
        const auto cp = value.codepoint;

        if (unicode::IsRegionalIndicator(cp))
            ++regional_indicators;

        if (unicode::IsEmojiPresentation(cp))
            has_default_emoji = true;

        if (unicode::IsExtendedPictographic(cp))
            has_extended_pictographic = true;

        if (unicode::IsEmojiModifier(cp))
            has_modifier = true;

        if (cp == 0xFE0F)
            has_vs16 = true;

        if (cp == 0x200D)
            has_zwj = true;

        offset += value.bytes_consumed;
    }

    // Emoji flag sequence: two Regional Indicators.
    if (regional_indicators == 2)
        return true;

    // Characters whose default presentation is emoji.
    if (has_default_emoji)
        return true;

    // Text-default pictograph explicitly switched to emoji
    // presentation using VS16.
    if (has_extended_pictographic && has_vs16)
        return true;

    // Emoji modifier sequence, e.g. 👍🏽.
    if (has_extended_pictographic && has_modifier)
        return true;

    // Extended pictographic ZWJ sequence,
    // e.g. family emoji.
    if (has_extended_pictographic && has_zwj)
        return true;

    return false;
}

} // namespace

GraphemeClass GraphemeClassifier::Classify(
    std::string_view grapheme
) noexcept
{
    if (grapheme.empty())
        return GraphemeClass::Other;

    if (IsEmojiGrapheme(grapheme))
        return GraphemeClass::Emoji;

    usize offset = 0;

    bool has_letter = false;
    bool has_number = false;
    bool has_punctuation = false;
    bool has_symbol = false;
    bool has_whitespace = false;
    bool has_control = false;
    bool has_non_mark = false;

    while (offset < grapheme.size())
    {
        auto decoded =
            unicode::Utf8::Decode(
                grapheme.substr(offset)
            );

        if (decoded.Failed())
            return GraphemeClass::Other;

        const auto value = decoded.Value();
        const auto cp = value.codepoint;

        const auto category =
            unicode::GeneralCategoryOf(cp);

        if (IsMark(category))
        {
            offset += value.bytes_consumed;
            continue;
        }

        has_non_mark = true;

        if (IsWhitespace(category, cp))
            has_whitespace = true;
        else if (IsControl(category))
            has_control = true;
        else if (IsLetter(category))
            has_letter = true;
        else if (IsNumber(category))
            has_number = true;
        else if (IsPunctuation(category))
            has_punctuation = true;
        else if (IsSymbol(category))
            has_symbol = true;

        offset += value.bytes_consumed;
    }

    if (has_control)
        return GraphemeClass::Control;

    if (has_whitespace)
        return GraphemeClass::Whitespace;

    if (has_letter)
        return GraphemeClass::Letter;

    if (has_number)
        return GraphemeClass::Number;

    if (has_punctuation)
        return GraphemeClass::Punctuation;

    if (has_symbol)
        return GraphemeClass::Symbol;

    if (!has_non_mark)
        return GraphemeClass::Other;

    return GraphemeClass::Other;
}

} // namespace qualix::pretokenizer
