#pragma once

#include <string_view>

#include "rules/rule_match.hpp"

namespace qualix::rules
{

class Rule
{
public:
    virtual ~Rule() = default;

    [[nodiscard]]
    virtual RuleType Type() const noexcept = 0;

    [[nodiscard]]
    virtual RuleMatch Match(
        std::string_view input,
        usize byte_offset
    ) const noexcept = 0;
};

} // namespace qualix::rules
