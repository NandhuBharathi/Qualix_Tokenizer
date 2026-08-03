#pragma once

#include <optional>
#include <span>
#include <unordered_map>

#include "bpe/pair.hpp"
#include "core/types.hpp"

namespace qualix::bpe
{

using PairFrequencies =
    std::unordered_map<
        Pair,
        usize,
        PairHash
    >;

struct PairFrequency
{
    Pair pair{};
    usize frequency = 0;
};

class PairCounter
{
public:
    [[nodiscard]]
    static PairFrequencies Count(
        std::span<const SymbolId> symbols
    );

    [[nodiscard]]
    static std::optional<PairFrequency> MostFrequent(
        const PairFrequencies& frequencies
    ) noexcept;
};

} // namespace qualix::bpe
