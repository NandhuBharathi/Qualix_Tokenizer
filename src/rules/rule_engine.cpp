#include "rules/rule_engine.hpp"

#include <utility>

namespace qualix::rules
{

void RuleEngine::Add(
    std::unique_ptr<Rule> rule
)
{
    if (rule)
        rules_.push_back(
            std::move(rule)
        );
}

RuleMatch RuleEngine::MatchAt(
    std::string_view input,
    usize byte_offset
) const noexcept
{
    if (byte_offset >= input.size())
        return {};

    RuleMatch best{};

    for (const auto& rule : rules_)
    {
        const RuleMatch match =
            rule->Match(
                input,
                byte_offset
            );

        if (!match.Matched())
            continue;

        if (match.byte_start != byte_offset)
            continue;

        if (match.ByteEnd() > input.size())
            continue;

        if (!best.Matched() ||
            match.byte_length >
                best.byte_length)
        {
            best = match;
        }
    }

    return best;
}

} // namespace qualix::rules
