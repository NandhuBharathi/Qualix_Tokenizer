#pragma once

#include "core/types.hpp"

namespace qualix::unicode
{

using CodePoint = u32;

inline constexpr CodePoint MaxCodePoint = 0x10FFFF;
inline constexpr CodePoint HighSurrogateStart = 0xD800;
inline constexpr CodePoint HighSurrogateEnd = 0xDBFF;
inline constexpr CodePoint LowSurrogateStart = 0xDC00;
inline constexpr CodePoint LowSurrogateEnd = 0xDFFF;

[[nodiscard]]
constexpr bool IsSurrogate(CodePoint cp) noexcept
{
    return cp >= HighSurrogateStart && cp <= LowSurrogateEnd;
}

[[nodiscard]]
constexpr bool IsValidCodePoint(CodePoint cp) noexcept
{
    return cp <= MaxCodePoint && !IsSurrogate(cp);
}

[[nodiscard]]
constexpr bool IsAscii(CodePoint cp) noexcept
{
    return cp <= 0x7F;
}

} // namespace qualix::unicode
