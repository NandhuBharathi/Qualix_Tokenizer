#pragma once

#include <array>
#include <cstddef>
#include <span>
#include <vector>

#include "core/types.hpp"

namespace qualix::bpe
{

inline constexpr std::array<u8, 8>
    BpeBinaryMagic{
        'Q', 'L', 'X', 'B',
        'P', 'E', '\r', '\n'
    };

inline constexpr u32
    BpeBinaryVersion = 1;

inline constexpr u32
    BpeBinaryFlags = 0;

class BpeBinaryWriter
{
public:
    void WriteU32(
        u32 value
    );

    void WriteU64(
        u64 value
    );

    void WriteBytes(
        std::span<const u8> bytes
    );

    [[nodiscard]]
    const std::vector<u8>& Data()
        const noexcept;

    [[nodiscard]]
    std::vector<u8> Take();

private:
    std::vector<u8> data_;
};

class BpeBinaryReader
{
public:
    explicit BpeBinaryReader(
        std::span<const u8> data
    ) noexcept;

    [[nodiscard]]
    bool ReadU32(
        u32& value
    ) noexcept;

    [[nodiscard]]
    bool ReadU64(
        u64& value
    ) noexcept;

    [[nodiscard]]
    bool ReadBytes(
        usize count,
        std::span<const u8>& bytes
    ) noexcept;

    [[nodiscard]]
    usize Position() const noexcept;

    [[nodiscard]]
    usize Remaining() const noexcept;

    [[nodiscard]]
    bool End() const noexcept;

private:
    std::span<const u8> data_;
    usize position_ = 0;
};

} // namespace qualix::bpe
