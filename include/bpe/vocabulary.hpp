#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "bpe/symbol.hpp"
#include "core/types.hpp"

namespace qualix::bpe
{

class Vocabulary
{
public:
    Vocabulary();

    [[nodiscard]]
    std::optional<SymbolId> Find(
        std::string_view symbol
    ) const noexcept;

    [[nodiscard]]
    std::optional<std::string_view> Find(
        SymbolId id
    ) const noexcept;

    [[nodiscard]]
    SymbolId Add(
        std::string symbol
    );


    [[nodiscard]]
    bool AddWithId(
        SymbolId id,
        std::string symbol
    );

    [[nodiscard]]
    SymbolId AddMerged(
        SymbolId left,
        SymbolId right
    );

    /*
     * Register the complete 256-byte fallback
     * alphabet in this vocabulary.
     *
     * Registration is append-only and idempotent.
     * Existing symbol IDs are never shifted.
     */
    [[nodiscard]]
    bool EnsureByteFallback();

    [[nodiscard]]
    bool HasByteFallback() const noexcept;

    [[nodiscard]]
    std::optional<SymbolId> FindByte(
        u8 byte
    ) const noexcept;

    [[nodiscard]]
    bool Contains(
        std::string_view symbol
    ) const noexcept;

    [[nodiscard]]
    bool Contains(
        SymbolId id
    ) const noexcept;

    [[nodiscard]]
    usize Size() const noexcept;

    [[nodiscard]]
    SymbolId NextId() const noexcept;

private:
    std::vector<std::string> id_to_symbol_;

    std::unordered_map<
        std::string,
        SymbolId
    > symbol_to_id_;
};

} // namespace qualix::bpe
