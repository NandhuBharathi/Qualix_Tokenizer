#pragma once

#include "rules/number_rule.hpp"
#include "rules/rule.hpp"

namespace qualix::rules
{

class PercentageRule final : public Rule
{
public:
    [[nodiscard]]
    RuleType Type() const noexcept override
    {
        return RuleType::Percentage;
    }

    [[nodiscard]]
    RuleMatch Match(
        std::string_view input,
        usize byte_offset
    ) const noexcept override;

private:
    NumberRule number_rule_;
};

} // namespace qualix::rules
