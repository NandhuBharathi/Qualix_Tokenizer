#pragma once

#include <string_view>
#include <vector>

#include "core/result.hpp"
#include "pretokenizer/span.hpp"

namespace qualix::pretokenizer
{

class PreTokenizer
{
public:
    static Result<std::vector<Span>> Split(
        std::string_view input
    );
};

} // namespace qualix::pretokenizer
