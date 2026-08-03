#pragma once

#include <optional>

#include "bpe/symbol.hpp"

namespace qualix::bpe
{

class SymbolAllocator
{
public:
    explicit constexpr SymbolAllocator(
        SymbolId first_available
    ) noexcept
        : next_(first_available)
    {
    }

    [[nodiscard]]
    constexpr SymbolId Next() const noexcept
    {
        return next_;
    }

    [[nodiscard]]
    std::optional<SymbolId> Allocate() noexcept;

private:
    SymbolId next_ = InvalidSymbolId;
};

} // namespace qualix::bpe
