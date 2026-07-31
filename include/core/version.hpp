#pragma once

#include <string_view>

#include "core/types.hpp"

namespace qualix
{

struct Version
{
    static constexpr u32 Major = 0;
    static constexpr u32 Minor = 1;
    static constexpr u32 Patch = 0;

    static constexpr std::string_view Name = "Qualix Tokenizer";

    static constexpr std::string_view String() noexcept
    {
        return "0.1.0";
    }
};

} // namespace qualix
