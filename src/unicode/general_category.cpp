#include "unicode/general_category.hpp"

#include <algorithm>

#include "unicode/generated/general_category_tables.hpp"

namespace qualix::unicode
{

GeneralCategory GeneralCategoryOf(
    CodePoint codepoint
) noexcept
{
    using generated::GeneralCategoryRanges;

    auto it = std::lower_bound(
        GeneralCategoryRanges.begin(),
        GeneralCategoryRanges.end(),
        codepoint,
        [](const auto& range, CodePoint value)
        {
            return range.last < value;
        }
    );

    if (it == GeneralCategoryRanges.end())
        return GeneralCategory::Cn;

    if (codepoint < it->first ||
        codepoint > it->last)
    {
        return GeneralCategory::Cn;
    }

    return it->category;
}

} // namespace qualix::unicode
