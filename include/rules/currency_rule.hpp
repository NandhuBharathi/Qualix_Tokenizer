#pragma once

#include "rules/numeric_scanner.hpp"
#include "rules/rule.hpp"

namespace qualix::rules
{

class CurrencyRule final : public Rule
{
public:
    [[nodiscard]]
    RuleType Type() const noexcept override
    {
        return RuleType::Currency;
    }

    [[nodiscard]]
    RuleMatch Match(
        std::string_view input,
        usize byte_offset
    ) const noexcept override;

};

} // namespace qualix::rules
