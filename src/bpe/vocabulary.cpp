#include "bpe/vocabulary.hpp"

#include "bpe/byte_fallback.hpp"

#include <utility>

namespace qualix::bpe
{

Vocabulary::Vocabulary()
{
    /*
     * Symbol ID 0 is permanently reserved as
     * InvalidSymbolId.
     */
    id_to_symbol_.emplace_back();
}

std::optional<SymbolId>
Vocabulary::Find(
    std::string_view symbol
) const noexcept
{
    const auto it =
        symbol_to_id_.find(
            std::string(symbol)
        );

    if (it == symbol_to_id_.end())
        return std::nullopt;

    return it->second;
}

std::optional<std::string_view>
Vocabulary::Find(
    SymbolId id
) const noexcept
{
    if (id == InvalidSymbolId)
        return std::nullopt;

    if (static_cast<usize>(id) >=
        id_to_symbol_.size())
    {
        return std::nullopt;
    }

    return std::string_view{
        id_to_symbol_[id]
    };
}

SymbolId Vocabulary::Add(
    std::string symbol
)
{
    if (symbol.empty())
        return InvalidSymbolId;

    const auto existing =
        symbol_to_id_.find(symbol);

    if (existing != symbol_to_id_.end())
        return existing->second;

    const SymbolId id =
        static_cast<SymbolId>(
            id_to_symbol_.size()
        );

    id_to_symbol_.push_back(symbol);

    symbol_to_id_.emplace(
        std::move(symbol),
        id
    );

    return id;
}


bool Vocabulary::AddWithId(
    SymbolId id,
    std::string symbol
)
{
    /*
     * ID zero is permanently reserved.
     */
    if (id == InvalidSymbolId ||
        symbol.empty())
    {
        return false;
    }

    /*
     * Duplicate textual symbols are only valid
     * when they already map to this exact ID.
     */
    const auto existing_symbol =
        symbol_to_id_.find(symbol);

    if (existing_symbol !=
        symbol_to_id_.end())
    {
        return
            existing_symbol->second ==
            id;
    }

    /*
     * Existing occupied ID.
     *
     * Re-importing the exact same mapping is
     * idempotent. Replacing it with different
     * text is forbidden.
     */
    if (static_cast<usize>(id) <
        id_to_symbol_.size())
    {
        if (id_to_symbol_[id].empty())
        {
            id_to_symbol_[id] =
                symbol;

            symbol_to_id_.emplace(
                std::move(symbol),
                id
            );

            return true;
        }

        return
            id_to_symbol_[id] ==
            symbol;
    }

    /*
     * Exact-ID import is intentionally strict:
     * no holes are allowed in the vocabulary.
     *
     * Serialized Qualix vocabularies therefore
     * remain dense:
     *
     * 1, 2, 3, ... N
     */
    if (id != NextId())
        return false;

    id_to_symbol_.push_back(
        symbol
    );

    symbol_to_id_.emplace(
        std::move(symbol),
        id
    );

    return true;
}

SymbolId Vocabulary::AddMerged(
    SymbolId left,
    SymbolId right
)
{
    const auto left_symbol =
        Find(left);

    if (!left_symbol.has_value())
        return InvalidSymbolId;

    const auto right_symbol =
        Find(right);

    if (!right_symbol.has_value())
        return InvalidSymbolId;

    std::string merged;

    merged.reserve(
        left_symbol->size() +
        right_symbol->size()
    );

    merged.append(
        left_symbol->data(),
        left_symbol->size()
    );

    merged.append(
        right_symbol->data(),
        right_symbol->size()
    );

    return Add(
        std::move(merged)
    );
}

bool Vocabulary::EnsureByteFallback()
{
    /*
     * Preflight the complete namespace before
     * mutating the vocabulary.
     *
     * A textual collision such as "<0x41>"
     * created by ordinary vocabulary training
     * must not silently become a fallback token.
     *
     * We only accept:
     *
     *   - none of the byte symbols exist, or
     *   - all 256 already exist contiguously
     *
     * Partial pre-existing fallback alphabets are
     * rejected to keep registration atomic from
     * the caller's point of view.
     */
    usize existing_count = 0;
    SymbolId existing_base =
        InvalidSymbolId;

    for (usize i = 0;
         i < ByteFallback::ByteCount;
         ++i)
    {
        const auto existing =
            Find(
                ByteFallback::Symbol(
                    static_cast<u8>(i)
                )
            );

        if (!existing.has_value())
            continue;

        if (existing_count == 0)
        {
            existing_base =
                *existing;
        }

        const SymbolId expected =
            static_cast<SymbolId>(
                existing_base +
                existing_count
            );

        if (*existing != expected)
            return false;

        ++existing_count;
    }

    if (existing_count != 0 &&
        existing_count !=
            ByteFallback::ByteCount)
    {
        return false;
    }

    /*
     * Complete existing block:
     * verify exact byte ordering.
     */
    if (existing_count ==
        ByteFallback::ByteCount)
    {
        for (usize i = 0;
             i < ByteFallback::ByteCount;
             ++i)
        {
            const auto id =
                Find(
                    ByteFallback::Symbol(
                        static_cast<u8>(i)
                    )
                );

            if (!id.has_value() ||
                *id !=
                    static_cast<SymbolId>(
                        existing_base + i
                    ))
            {
                return false;
            }
        }

        return true;
    }

    /*
     * No fallback symbols currently exist.
     * Append all 256 in deterministic byte order.
     */
    for (usize i = 0;
         i < ByteFallback::ByteCount;
         ++i)
    {
        const std::string symbol =
            ByteFallback::Symbol(
                static_cast<u8>(i)
            );

        const SymbolId expected =
            NextId();

        const SymbolId id =
            Add(symbol);

        if (id == InvalidSymbolId ||
            id != expected)
        {
            return false;
        }
    }

    return true;
}

bool Vocabulary::HasByteFallback()
    const noexcept
{
    /*
     * Determine the base from byte zero and then
     * verify the complete contiguous block.
     */
    const auto first =
        Find(
            ByteFallback::Symbol(
                static_cast<u8>(0)
            )
        );

    if (!first.has_value())
        return false;

    for (usize i = 0;
         i < ByteFallback::ByteCount;
         ++i)
    {
        const auto id =
            Find(
                ByteFallback::Symbol(
                    static_cast<u8>(i)
                )
            );

        if (!id.has_value())
            return false;

        const SymbolId expected =
            static_cast<SymbolId>(
                *first + i
            );

        if (*id != expected)
            return false;
    }

    return true;
}

std::optional<SymbolId>
Vocabulary::FindByte(
    u8 byte
) const noexcept
{
    if (!HasByteFallback())
        return std::nullopt;

    return Find(
        ByteFallback::Symbol(byte)
    );
}

bool Vocabulary::Contains(
    std::string_view symbol
) const noexcept
{
    return Find(symbol).has_value();
}

bool Vocabulary::Contains(
    SymbolId id
) const noexcept
{
    return Find(id).has_value();
}

usize Vocabulary::Size() const noexcept
{
    /*
     * Slot 0 is reserved and therefore is not
     * part of the usable vocabulary.
     */
    return
        id_to_symbol_.empty()
            ? 0
            : id_to_symbol_.size() - 1;
}

SymbolId Vocabulary::NextId() const noexcept
{
    return static_cast<SymbolId>(
        id_to_symbol_.size()
    );
}

} // namespace qualix::bpe
