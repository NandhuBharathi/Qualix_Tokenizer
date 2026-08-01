#include "unicode/indic_conjunct.hpp"
#include "test_framework.hpp"

using namespace qualix::unicode;
using namespace qualix::test;

int main()
{
    Expect(
        ToString(IndicConjunctBreak::None) == "None",
        "InCB None string"
    );

    Expect(
        ToString(IndicConjunctBreak::Consonant) == "Consonant",
        "InCB Consonant string"
    );

    Expect(
        ToString(IndicConjunctBreak::Extend) == "Extend",
        "InCB Extend string"
    );

    Expect(
        ToString(IndicConjunctBreak::Linker) == "Linker",
        "InCB Linker string"
    );

    Expect(
        GetIndicConjunctBreak(0x0915) ==
            IndicConjunctBreak::Consonant,
        "Devanagari KA consonant"
    );

    Expect(
        GetIndicConjunctBreak(0x094D) ==
            IndicConjunctBreak::Linker,
        "Devanagari Virama linker"
    );

    Expect(
        GetIndicConjunctBreak(0x093C) ==
            IndicConjunctBreak::Extend,
        "Devanagari Nukta extend"
    );

    Expect(
        GetIndicConjunctBreak(0x0041) ==
            IndicConjunctBreak::None,
        "ASCII has no InCB"
    );

    return Summary();
}
