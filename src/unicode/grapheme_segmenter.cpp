#include "unicode/grapheme_segmenter.hpp"

#include <vector>

#include "unicode/grapheme_property.hpp"
#include "unicode/indic_conjunct.hpp"
#include "unicode/utf8.hpp"

namespace qualix::unicode
{

namespace
{

struct Unit
{
    CodePoint cp;
    usize byte_start;
    usize byte_length;
    GraphemeBreakProperty gcb;
    IndicConjunctBreak incb;
    bool extended_pictographic;
};

bool IsControlLike(GraphemeBreakProperty p) noexcept
{
    return p == GraphemeBreakProperty::CR ||
           p == GraphemeBreakProperty::LF ||
           p == GraphemeBreakProperty::Control;
}

bool HasIndicConjunctBefore(
    const std::vector<Unit>& units,
    usize boundary
) noexcept
{
    if (boundary == 0)
        return false;

    if (units[boundary].incb != IndicConjunctBreak::Consonant)
        return false;

    usize i = boundary;

    bool linker_seen = false;

    while (i > 0)
    {
        --i;

        const auto property = units[i].incb;

        if (property == IndicConjunctBreak::Linker)
        {
            linker_seen = true;
            continue;
        }

        if (property == IndicConjunctBreak::Extend)
            continue;

        return linker_seen &&
               property == IndicConjunctBreak::Consonant;
    }

    return false;
}

bool HasExtendedPictographicBeforeZWJ(
    const std::vector<Unit>& units,
    usize boundary
) noexcept
{
    if (boundary < 2)
        return false;

    if (!units[boundary].extended_pictographic)
        return false;

    usize i = boundary - 1;

    if (units[i].gcb != GraphemeBreakProperty::ZWJ)
        return false;

    while (i > 0)
    {
        --i;

        if (units[i].gcb == GraphemeBreakProperty::Extend)
            continue;

        return units[i].extended_pictographic;
    }

    return false;
}

usize CountPreviousRegionalIndicators(
    const std::vector<Unit>& units,
    usize boundary
) noexcept
{
    usize count = 0;
    usize i = boundary;

    while (i > 0)
    {
        --i;

        if (units[i].gcb !=
            GraphemeBreakProperty::RegionalIndicator)
        {
            break;
        }

        ++count;
    }

    return count;
}

bool ShouldBreak(
    const std::vector<Unit>& units,
    usize boundary
) noexcept
{
    const auto& left = units[boundary - 1];
    const auto& right = units[boundary];

    const auto a = left.gcb;
    const auto b = right.gcb;

    // GB3
    if (a == GraphemeBreakProperty::CR &&
        b == GraphemeBreakProperty::LF)
    {
        return false;
    }

    // GB4
    if (IsControlLike(a))
        return true;

    // GB5
    if (IsControlLike(b))
        return true;

    // GB6
    if (a == GraphemeBreakProperty::L &&
        (b == GraphemeBreakProperty::L ||
         b == GraphemeBreakProperty::V ||
         b == GraphemeBreakProperty::LV ||
         b == GraphemeBreakProperty::LVT))
    {
        return false;
    }

    // GB7
    if ((a == GraphemeBreakProperty::LV ||
         a == GraphemeBreakProperty::V) &&
        (b == GraphemeBreakProperty::V ||
         b == GraphemeBreakProperty::T))
    {
        return false;
    }

    // GB8
    if ((a == GraphemeBreakProperty::LVT ||
         a == GraphemeBreakProperty::T) &&
        b == GraphemeBreakProperty::T)
    {
        return false;
    }

    // GB9
    if (b == GraphemeBreakProperty::Extend ||
        b == GraphemeBreakProperty::ZWJ)
    {
        return false;
    }

    // GB9a
    if (b == GraphemeBreakProperty::SpacingMark)
        return false;

    // GB9b
    if (a == GraphemeBreakProperty::Prepend)
        return false;

    // GB9c
    if (HasIndicConjunctBefore(units, boundary))
        return false;

    // GB11
    if (HasExtendedPictographicBeforeZWJ(units, boundary))
        return false;

    // GB12 / GB13
    if (a == GraphemeBreakProperty::RegionalIndicator &&
        b == GraphemeBreakProperty::RegionalIndicator)
    {
        const usize previous =
            CountPreviousRegionalIndicators(units, boundary);

        if ((previous % 2) == 1)
            return false;
    }

    // GB999
    return true;
}

} // namespace

Result<std::vector<Grapheme>> GraphemeSegmenter::Segment(
    std::string_view input
)
{
    std::vector<Grapheme> graphemes;

    if (input.empty())
        return graphemes;

    std::vector<Unit> units;
    units.reserve(input.size());

    usize offset = 0;

    while (offset < input.size())
    {
        auto decoded = Utf8::Decode(input.substr(offset));

        if (decoded.Failed())
            return Status::Failure(ErrorCode::InvalidUtf8);

        const auto value = decoded.Value();

        units.push_back(
            Unit{
                value.codepoint,
                offset,
                value.bytes_consumed,
                GetGraphemeBreakProperty(value.codepoint),
                GetIndicConjunctBreak(value.codepoint),
                IsExtendedPictographic(value.codepoint)
            }
        );

        offset += value.bytes_consumed;
    }

    if (units.empty())
        return graphemes;

    usize cluster_start = 0;

    for (usize i = 1; i < units.size(); ++i)
    {
        if (!ShouldBreak(units, i))
            continue;

        const usize byte_start =
            units[cluster_start].byte_start;

        const usize byte_end =
            units[i - 1].byte_start +
            units[i - 1].byte_length;

        graphemes.push_back(
            Grapheme{
                byte_start,
                byte_end - byte_start
            }
        );

        cluster_start = i;
    }

    const usize byte_start =
        units[cluster_start].byte_start;

    const usize byte_end =
        units.back().byte_start +
        units.back().byte_length;

    graphemes.push_back(
        Grapheme{
            byte_start,
            byte_end - byte_start
        }
    );

    return graphemes;
}

} // namespace qualix::unicode
