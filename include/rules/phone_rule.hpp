#pragma once

#include <string_view>

#include "rules/rule.hpp"

namespace qualix::rules
{

class PhoneRule final : public Rule
{
public:
    [[nodiscard]]
    RuleType Type() const noexcept override
    {
        return RuleType::Phone;
    }

    [[nodiscard]]
    RuleMatch Match(
        std::string_view input,
        usize byte_offset
    ) const noexcept override;
};

} // namespace qualix::rules
