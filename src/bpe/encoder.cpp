#include "bpe/encoder.hpp"

#include <algorithm>
#include <vector>

#include "bpe/merger.hpp"

namespace qualix::bpe
{

std::vector<SymbolId>
BpeEncoder::Encode(
    std::span<const SymbolId> symbols,
    std::span<const MergeRule> rules
)
{
    std::vector<SymbolId> output{
        symbols.begin(),
        symbols.end()
    };

    if (output.size() < 2 ||
        rules.empty())
    {
        return output;
    }

    /*
     * MergeRule::rank defines training order.
     *
     * Do not depend on the caller supplying
     * rules in sorted order.
     */
    std::vector<const MergeRule*>
        ordered_rules;

    ordered_rules.reserve(
        rules.size()
    );

    for (const auto& rule : rules)
    {
        if (rule.Valid())
            ordered_rules.push_back(
                &rule
            );
    }

    std::stable_sort(
        ordered_rules.begin(),
        ordered_rules.end(),
        [](
            const MergeRule* left,
            const MergeRule* right
        )
        {
            return
                left->rank <
                right->rank;
        }
    );

    for (const MergeRule* rule :
         ordered_rules)
    {
        if (output.size() < 2)
            break;

        output =
            Merger::Merge(
                output,
                rule->pair,
                rule->merged_symbol
            );
    }

    return output;
}

} // namespace qualix::bpe
