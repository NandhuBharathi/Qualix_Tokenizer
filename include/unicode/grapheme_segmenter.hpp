#pragma once

#include <string_view>
#include <vector>

#include "core/result.hpp"
#include "unicode/grapheme.hpp"

namespace qualix::unicode
{

class GraphemeSegmenter
{
public:
    [[nodiscard]]
    static Result<std::vector<Grapheme>> Segment(
        std::string_view input
    );
};

} // namespace qualix::unicode
