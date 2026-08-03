#pragma once

#include <array>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "core/types.hpp"

namespace qualix::bpe
{

/*
 * Universal byte fallback foundation.
 *
 * Every possible byte value has a deterministic
 * fallback symbol representation:
 *
 *   <0x00>
 *   <0x01>
 *   ...
 *   <0xFF>
 *
 * This representation is independent of language
 * and therefore can preserve arbitrary UTF-8 input
 * without requiring the script to have appeared
 * during BPE training.
 */
class ByteFallback
{
public:
    static constexpr usize ByteCount = 256;

    [[nodiscard]]
    static std::string Symbol(
        u8 byte
    );

    [[nodiscard]]
    static bool ParseSymbol(
        std::string_view symbol,
        u8& byte
    ) noexcept;

    [[nodiscard]]
    static std::vector<std::string> Encode(
        std::string_view text
    );

    [[nodiscard]]
    static std::string Decode(
        std::span<const std::string> symbols
    );

private:
    [[nodiscard]]
    static int HexValue(
        char value
    ) noexcept;
};

} // namespace qualix::bpe
