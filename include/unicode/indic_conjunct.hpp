#pragma once

#include <string_view>

#include "unicode/codepoint.hpp"

namespace qualix::unicode
{

enum class IndicConjunctBreak
{
    None,
    Consonant,
    Extend,
    Linker
};

[[nodiscard]]
IndicConjunctBreak GetIndicConjunctBreak(
    CodePoint codepoint
) noexcept;

[[nodiscard]]
constexpr std::string_view ToString(
    IndicConjunctBreak property
) noexcept
{
    switch (property)
    {
        case IndicConjunctBreak::None:
            return "None";
        case IndicConjunctBreak::Consonant:
            return "Consonant";
        case IndicConjunctBreak::Extend:
            return "Extend";
        case IndicConjunctBreak::Linker:
            return "Linker";
    }

    return "None";
}

} // namespace qualix::unicode
