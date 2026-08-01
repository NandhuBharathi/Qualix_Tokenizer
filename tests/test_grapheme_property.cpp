#include "unicode/grapheme_property.hpp"
#include "unicode/generated/grapheme_tables.hpp"
#include "test_framework.hpp"

using namespace qualix::unicode;
using namespace qualix::test;

int main()
{
    Expect(
        generated::UnicodeVersion == "17.0.0",
        "Unicode version 17.0.0"
    );

    Expect(
        GetGraphemeBreakProperty(0x000D) ==
            GraphemeBreakProperty::CR,
        "CR property"
    );

    Expect(
        GetGraphemeBreakProperty(0x000A) ==
            GraphemeBreakProperty::LF,
        "LF property"
    );

    Expect(
        GetGraphemeBreakProperty(0x0000) ==
            GraphemeBreakProperty::Control,
        "Control property"
    );

    Expect(
        GetGraphemeBreakProperty(0x0301) ==
            GraphemeBreakProperty::Extend,
        "Combining acute Extend"
    );

    Expect(
        GetGraphemeBreakProperty(0x200D) ==
            GraphemeBreakProperty::ZWJ,
        "ZWJ property"
    );

    Expect(
        GetGraphemeBreakProperty(0x1F1EE) ==
            GraphemeBreakProperty::RegionalIndicator,
        "Regional Indicator property"
    );

    Expect(
        GetGraphemeBreakProperty(0x0600) ==
            GraphemeBreakProperty::Prepend,
        "Prepend property"
    );

    Expect(
        GetGraphemeBreakProperty(0x0903) ==
            GraphemeBreakProperty::SpacingMark,
        "SpacingMark property"
    );

    Expect(
        GetGraphemeBreakProperty(0x1100) ==
            GraphemeBreakProperty::L,
        "Hangul L property"
    );

    Expect(
        GetGraphemeBreakProperty(0x1161) ==
            GraphemeBreakProperty::V,
        "Hangul V property"
    );

    Expect(
        GetGraphemeBreakProperty(0x11A8) ==
            GraphemeBreakProperty::T,
        "Hangul T property"
    );

    Expect(
        GetGraphemeBreakProperty(0xAC00) ==
            GraphemeBreakProperty::LV,
        "Hangul LV property"
    );

    Expect(
        GetGraphemeBreakProperty(0xAC01) ==
            GraphemeBreakProperty::LVT,
        "Hangul LVT property"
    );

    Expect(
        GetGraphemeBreakProperty(0x0041) ==
            GraphemeBreakProperty::Other,
        "ASCII A Other property"
    );

    Expect(
        IsExtendedPictographic(0x1F600),
        "Emoji Extended Pictographic"
    );

    Expect(
        IsExtendedPictographic(0x1F469),
        "Woman Extended Pictographic"
    );

    Expect(
        !IsExtendedPictographic(0x0041),
        "ASCII not Extended Pictographic"
    );

    Expect(
        !IsExtendedPictographic(0x0B85),
        "Tamil letter not Extended Pictographic"
    );

    return Summary();
}
