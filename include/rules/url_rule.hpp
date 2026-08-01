#pragma once

#include "rules/rule.hpp"

namespace qualix::rules
{

class UrlRule final : public Rule
{
public:
    [[nodiscard]]
    RuleType Type() const noexcept override
    {
        return RuleType::Url;
    }

    [[nodiscard]]
    RuleMatch Match(
        std::string_view input,
        usize byte_offset
    ) const noexcept override;
};

} // namespace qualix::rules
