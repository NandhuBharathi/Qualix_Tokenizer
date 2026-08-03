#include "bpe/merger.hpp"

namespace qualix::bpe
{

std::vector<SymbolId> Merger::Merge(
    std::span<const SymbolId> symbols,
    Pair pair,
    SymbolId merged_symbol
)
{
    if (symbols.empty())
        return {};

    if (!pair.Valid() ||
        merged_symbol == InvalidSymbolId)
    {
        return {
            symbols.begin(),
            symbols.end()
        };
    }

    std::vector<SymbolId> result;

    result.reserve(symbols.size());

    usize i = 0;

    while (i < symbols.size())
    {
        if (
            i + 1 < symbols.size() &&
            symbols[i] == pair.left &&
            symbols[i + 1] == pair.right
        )
        {
            result.push_back(
                merged_symbol
            );

            i += 2;
            continue;
        }

        result.push_back(
            symbols[i]
        );

        ++i;
    }

    return result;
}

} // namespace qualix::bpe
