#pragma once

#include "bpe/pair.hpp"
#include "bpe/symbol.hpp"
#include "core/types.hpp"

namespace qualix::bpe
{

using MergeRank = usize;

struct MergeRule
{
    Pair pair{};
    SymbolId merged_symbol = InvalidSymbolId;
    MergeRank rank = 0;

    constexpr bool Valid() const noexcept
    {
        return
            pair.Valid() &&
            merged_symbol != InvalidSymbolId;
    }

    constexpr bool operator==(
        const MergeRule& other
    ) const noexcept = default;
};

} // namespace qualix::bpe
