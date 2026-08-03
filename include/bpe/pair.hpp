#pragma once

#include <cstddef>
#include <functional>

#include "bpe/symbol.hpp"

namespace qualix::bpe
{

struct Pair
{
    SymbolId left = InvalidSymbolId;
    SymbolId right = InvalidSymbolId;

    constexpr bool Valid() const noexcept
    {
        return
            left != InvalidSymbolId &&
            right != InvalidSymbolId;
    }

    constexpr bool operator==(
        const Pair& other
    ) const noexcept = default;
};

struct PairHash
{
    std::size_t operator()(
        const Pair& pair
    ) const noexcept
    {
        const std::size_t left =
            std::hash<SymbolId>{}(pair.left);

        const std::size_t right =
            std::hash<SymbolId>{}(pair.right);

        return
            left ^
            (
                right +
                0x9e3779b9u +
                (left << 6u) +
                (left >> 2u)
            );
    }
};

} // namespace qualix::bpe
