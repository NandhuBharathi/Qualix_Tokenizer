#pragma once

#include <string>
#include <string_view>

#include "core/result.hpp"
#include "core/types.hpp"
#include "unicode/codepoint.hpp"

namespace qualix::unicode
{

struct Utf8DecodeResult
{
    CodePoint codepoint = 0;
    usize bytes_consumed = 0;
};

class Utf8
{
public:
    [[nodiscard]]
    static constexpr bool IsContinuationByte(u8 byte) noexcept
    {
        return (byte & 0xC0) == 0x80;
    }

    [[nodiscard]]
    static usize SequenceLength(u8 lead_byte) noexcept;

    [[nodiscard]]
    static bool Validate(std::string_view input) noexcept;

    [[nodiscard]]
    static Result<Utf8DecodeResult> Decode(std::string_view input) noexcept;

    [[nodiscard]]
    static Result<std::string> Encode(CodePoint codepoint);
};

} // namespace qualix::unicode
