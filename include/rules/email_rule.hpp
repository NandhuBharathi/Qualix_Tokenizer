#pragma once

#include "rules/rule.hpp"

namespace qualix::rules
{

class EmailRule final : public Rule
{
public:
    [[nodiscard]]
    RuleType Type() const noexcept override
    {
        return RuleType::Email;
    }

    [[nodiscard]]
    RuleMatch Match(
        std::string_view input,
        usize byte_offset
    ) const noexcept override;
};

} // namespace qualix::rules
