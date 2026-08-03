#pragma once

#include "rules/numeric_scanner.hpp"
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

};

} // namespace qualix::rules
