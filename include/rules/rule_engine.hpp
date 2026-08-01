#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "rules/rule.hpp"

namespace qualix::rules
{

class RuleEngine
{
public:
    RuleEngine() = default;

    void Add(
        std::unique_ptr<Rule> rule
    );

    [[nodiscard]]
    RuleMatch MatchAt(
        std::string_view input,
        usize byte_offset
    ) const noexcept;

    [[nodiscard]]
    usize RuleCount() const noexcept
    {
        return rules_.size();
    }

    [[nodiscard]]
    bool Empty() const noexcept
    {
        return rules_.empty();
    }

private:
    std::vector<std::unique_ptr<Rule>> rules_;
};

} // namespace qualix::rules
