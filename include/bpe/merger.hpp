#pragma once

#include <span>
#include <vector>

#include "bpe/pair.hpp"
#include "bpe/symbol.hpp"

namespace qualix::bpe
{

class Merger
{
public:
    [[nodiscard]]
    static std::vector<SymbolId> Merge(
        std::span<const SymbolId> symbols,
        Pair pair,
        SymbolId merged_symbol
    );
};

} // namespace qualix::bpe
