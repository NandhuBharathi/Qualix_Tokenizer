#pragma once

#include <span>
#include <vector>

#include "bpe/merge_rule.hpp"
#include "bpe/symbol.hpp"

namespace qualix::bpe
{

class BpeEncoder
{
public:
    [[nodiscard]]
    static std::vector<SymbolId> Encode(
        std::span<const SymbolId> symbols,
        std::span<const MergeRule> rules
    );
};

} // namespace qualix::bpe
