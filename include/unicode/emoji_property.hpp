#pragma once

#include "unicode/codepoint.hpp"

namespace qualix::unicode
{

[[nodiscard]]
bool IsEmoji(
    CodePoint codepoint
) noexcept;

[[nodiscard]]
bool IsEmojiPresentation(
    CodePoint codepoint
) noexcept;

[[nodiscard]]
bool IsEmojiModifier(
    CodePoint codepoint
) noexcept;

[[nodiscard]]
bool IsEmojiModifierBase(
    CodePoint codepoint
) noexcept;

[[nodiscard]]
bool IsRegionalIndicator(
    CodePoint codepoint
) noexcept;

} // namespace qualix::unicode
