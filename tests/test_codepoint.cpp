#include "unicode/codepoint.hpp"
#include "test_framework.hpp"

using namespace qualix::unicode;
using namespace qualix::test;

int main()
{
    Expect(IsValidCodePoint(0x0000), "U+0000 valid");
    Expect(IsValidCodePoint(0x0041), "ASCII A valid");
    Expect(IsValidCodePoint(0x0B85), "Tamil A valid");
    Expect(IsValidCodePoint(0x1F600), "Emoji valid");
    Expect(IsValidCodePoint(0x10FFFF), "Maximum code point valid");

    Expect(!IsValidCodePoint(0xD800), "High surrogate start invalid");
    Expect(!IsValidCodePoint(0xDBFF), "High surrogate end invalid");
    Expect(!IsValidCodePoint(0xDC00), "Low surrogate start invalid");
    Expect(!IsValidCodePoint(0xDFFF), "Low surrogate end invalid");
    Expect(!IsValidCodePoint(0x110000), "Above Unicode maximum invalid");

    Expect(IsSurrogate(0xD800), "Detect surrogate start");
    Expect(IsSurrogate(0xDFFF), "Detect surrogate end");
    Expect(!IsSurrogate(0xD7FF), "Before surrogate range");
    Expect(!IsSurrogate(0xE000), "After surrogate range");

    Expect(IsAscii(0x00), "ASCII minimum");
    Expect(IsAscii(0x7F), "ASCII maximum");
    Expect(!IsAscii(0x80), "Non ASCII");

    return Summary();
}
