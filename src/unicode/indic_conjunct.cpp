#include "unicode/indic_conjunct.hpp"
#include "unicode/generated/indic_conjunct_table.hpp"

namespace qualix::unicode
{

IndicConjunctBreak GetIndicConjunctBreak(
    CodePoint codepoint
) noexcept
{
    const auto& ranges = generated::IndicConjunctRanges;

    usize left = 0;
    usize right = ranges.size();

    while (left < right)
    {
        const usize middle =
            left + (right - left) / 2;

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

    return IndicConjunctBreak::None;
}

} // namespace qualix::unicode
