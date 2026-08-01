#pragma once

#include <string_view>

#include "pretokenizer/grapheme_class.hpp"

namespace qualix::pretokenizer
{

class GraphemeClassifier
{
public:
    static GraphemeClass Classify(
        std::string_view grapheme
    ) noexcept;
};

} // namespace qualix::pretokenizer
