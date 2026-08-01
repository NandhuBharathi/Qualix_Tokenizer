#pragma once

#include <string_view>

#include "core/types.hpp"
#include "rules/rule_type.hpp"

namespace qualix::rules
{

struct RuleMatch
{
    RuleType type{RuleType::None};

    usize byte_start{0};
    usize byte_length{0};

    [[nodiscard]]
    constexpr usize ByteEnd() const noexcept
    {
        return byte_start + byte_length;
    }

    [[nodiscard]]
    constexpr bool Empty() const noexcept
    {
        return byte_length == 0;
    }

    [[nodiscard]]
    constexpr bool Matched() const noexcept
    {
        return type != RuleType::None &&
               byte_length != 0;
    }

    [[nodiscard]]
    std::string_view View(
        std::string_view text
    ) const noexcept
    {
        if (byte_start > text.size())
            return {};

        if (byte_length >
            text.size() - byte_start)
        {
            return {};
        }

        return text.substr(
            byte_start,
            byte_length
        );
    }
};

} // namespace qualix::rules
