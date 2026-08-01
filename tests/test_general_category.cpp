#include "unicode/general_category.hpp"
#include "test_framework.hpp"

using namespace qualix::unicode;
using namespace qualix::test;

int main()
{
    Expect(
        GeneralCategoryOf(0x0041) ==
            GeneralCategory::Lu,
        "Latin A uppercase letter"
    );

    Expect(
        GeneralCategoryOf(0x0061) ==
            GeneralCategory::Ll,
        "Latin a lowercase letter"
    );

    Expect(
        GeneralCategoryOf(0x0B95) ==
            GeneralCategory::Lo,
        "Tamil KA other letter"
    );

    Expect(
        GeneralCategoryOf(0x0915) ==
            GeneralCategory::Lo,
        "Devanagari KA other letter"
    );

    Expect(
        GeneralCategoryOf(0x4E2D) ==
            GeneralCategory::Lo,
        "CJK ideograph other letter"
    );

    Expect(
        GeneralCategoryOf(0x0301) ==
            GeneralCategory::Mn,
        "Combining acute nonspacing mark"
    );

    Expect(
        GeneralCategoryOf(0x0037) ==
            GeneralCategory::Nd,
        "ASCII digit decimal number"
    );

    Expect(
        GeneralCategoryOf(0x0021) ==
            GeneralCategory::Po,
        "Exclamation punctuation"
    );

    Expect(
        GeneralCategoryOf(0x20B9) ==
            GeneralCategory::Sc,
        "Rupee currency symbol"
    );

    Expect(
        GeneralCategoryOf(0x2211) ==
            GeneralCategory::Sm,
        "Summation math symbol"
    );

    Expect(
        GeneralCategoryOf(0x0020) ==
            GeneralCategory::Zs,
        "ASCII space separator"
    );

    Expect(
        GeneralCategoryOf(0x000A) ==
            GeneralCategory::Cc,
        "LF control"
    );

    Expect(
        GeneralCategoryOf(0xE000) ==
            GeneralCategory::Co,
        "Private use character"
    );

    Expect(
        GeneralCategoryOf(0x0378) ==
            GeneralCategory::Cn,
        "Unassigned codepoint"
    );

    Expect(
        GeneralCategoryOf(0x110000) ==
            GeneralCategory::Cn,
        "Out of Unicode range"
    );

    Expect(
        ToString(GeneralCategory::Lu) == "Lu",
        "Lu category string"
    );

    Expect(
        ToString(GeneralCategory::Sc) == "Sc",
        "Sc category string"
    );

    Expect(
        ToString(GeneralCategory::Cn) == "Cn",
        "Cn category string"
    );

    return Summary();
}
