#pragma once

#include "core/types.hpp"

namespace qualix::bpe
{

using SymbolId = u32;

inline constexpr SymbolId InvalidSymbolId = 0;

struct Symbol
{
    SymbolId id = InvalidSymbolId;

    constexpr bool Valid() const noexcept
    {
        return id != InvalidSymbolId;
    }

    constexpr bool operator==(
        const Symbol& other
    ) const noexcept = default;
};

} // namespace qualix::bpe
