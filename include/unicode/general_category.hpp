#pragma once

#include <string_view>

#include "unicode/codepoint.hpp"

namespace qualix::unicode
{

enum class GeneralCategory
{
    Cn = 0,

    Lu, Ll, Lt, Lm, Lo,
    Mn, Mc, Me,
    Nd, Nl, No,
    Pc, Pd, Ps, Pe, Pi, Pf, Po,
    Sm, Sc, Sk, So,
    Zs, Zl, Zp,
    Cc, Cf, Cs, Co
};

constexpr std::string_view ToString(
    GeneralCategory category
) noexcept
{
    switch (category)
    {
        case GeneralCategory::Cn: return "Cn";

        case GeneralCategory::Lu: return "Lu";
        case GeneralCategory::Ll: return "Ll";
        case GeneralCategory::Lt: return "Lt";
        case GeneralCategory::Lm: return "Lm";
        case GeneralCategory::Lo: return "Lo";

        case GeneralCategory::Mn: return "Mn";
        case GeneralCategory::Mc: return "Mc";
        case GeneralCategory::Me: return "Me";

        case GeneralCategory::Nd: return "Nd";
        case GeneralCategory::Nl: return "Nl";
        case GeneralCategory::No: return "No";

        case GeneralCategory::Pc: return "Pc";
        case GeneralCategory::Pd: return "Pd";
        case GeneralCategory::Ps: return "Ps";
        case GeneralCategory::Pe: return "Pe";
        case GeneralCategory::Pi: return "Pi";
        case GeneralCategory::Pf: return "Pf";
        case GeneralCategory::Po: return "Po";

        case GeneralCategory::Sm: return "Sm";
        case GeneralCategory::Sc: return "Sc";
        case GeneralCategory::Sk: return "Sk";
        case GeneralCategory::So: return "So";

        case GeneralCategory::Zs: return "Zs";
        case GeneralCategory::Zl: return "Zl";
        case GeneralCategory::Zp: return "Zp";

        case GeneralCategory::Cc: return "Cc";
        case GeneralCategory::Cf: return "Cf";
        case GeneralCategory::Cs: return "Cs";
        case GeneralCategory::Co: return "Co";
    }

    return "Cn";
}

GeneralCategory GeneralCategoryOf(
    CodePoint codepoint
) noexcept;

} // namespace qualix::unicode
