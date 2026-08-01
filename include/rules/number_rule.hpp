#pragma once

#include "rules/rule.hpp"

namespace qualix::rules
{

class NumberRule final : public Rule
{
public:
    [[nodiscard]]
    RuleType Type() const noexcept override
    {
        return RuleType::Number;
    }

    [[nodiscard]]
    RuleMatch Match(
        std::string_view input,
        usize byte_offset
    ) const noexcept override;
};

} // namespace qualix::rules
