#include <string>
#include <string_view>

#include "unicode/utf8.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::unicode;
using namespace qualix::test;

int main()
{
    Expect(Utf8::SequenceLength(0x41) == 1, "ASCII sequence length");
    Expect(Utf8::SequenceLength(0xC2) == 2, "2-byte sequence length");
    Expect(Utf8::SequenceLength(0xE0) == 3, "3-byte sequence length");
    Expect(Utf8::SequenceLength(0xF0) == 4, "4-byte sequence length");
    Expect(Utf8::SequenceLength(0x80) == 0, "Continuation byte not lead");
    Expect(Utf8::SequenceLength(0xC0) == 0, "Invalid overlong lead");
    Expect(Utf8::SequenceLength(0xF5) == 0, "Invalid high lead");

    Expect(Utf8::IsContinuationByte(0x80), "Continuation byte minimum");
    Expect(Utf8::IsContinuationByte(0xBF), "Continuation byte maximum");
    Expect(!Utf8::IsContinuationByte(0x7F), "ASCII not continuation");
    Expect(!Utf8::IsContinuationByte(0xC0), "Lead not continuation");

    auto ascii = Utf8::Decode("A");
    Expect(ascii.Ok(), "Decode ASCII");
    Expect(ascii.Value().codepoint == 0x41, "ASCII code point");
    Expect(ascii.Value().bytes_consumed == 1, "ASCII bytes consumed");

    auto two = Utf8::Decode("\xC2\xA2");
    Expect(two.Ok(), "Decode 2-byte character");
    Expect(two.Value().codepoint == 0x00A2, "2-byte code point");
    Expect(two.Value().bytes_consumed == 2, "2-byte consumed");

    auto tamil = Utf8::Decode("\xE0\xAE\x85");
    Expect(tamil.Ok(), "Decode Tamil character");
    Expect(tamil.Value().codepoint == 0x0B85, "Tamil code point");
    Expect(tamil.Value().bytes_consumed == 3, "Tamil bytes consumed");

    auto emoji = Utf8::Decode("\xF0\x9F\x98\x80");
    Expect(emoji.Ok(), "Decode emoji");
    Expect(emoji.Value().codepoint == 0x1F600, "Emoji code point");
    Expect(emoji.Value().bytes_consumed == 4, "Emoji bytes consumed");

    Expect(Utf8::Validate("Hello"), "Validate ASCII");
    Expect(Utf8::Validate("வணக்கம்"), "Validate Tamil");
    Expect(Utf8::Validate("हिन्दी"), "Validate Hindi");
    Expect(Utf8::Validate("中文"), "Validate Chinese");
    Expect(Utf8::Validate("한국어"), "Validate Korean");
    Expect(Utf8::Validate("العربية"), "Validate Arabic");
    Expect(Utf8::Validate("😀👍🏽"), "Validate emoji");
    Expect(Utf8::Validate("👨‍👩‍👧‍👦"), "Validate ZWJ emoji sequence");

    Expect(!Utf8::Validate(std::string("\x80", 1)),
           "Reject isolated continuation byte");

    Expect(!Utf8::Validate(std::string("\xC0\x80", 2)),
           "Reject 2-byte overlong sequence");

    Expect(!Utf8::Validate(std::string("\xE0\x80\x80", 3)),
           "Reject 3-byte overlong sequence");

    Expect(!Utf8::Validate(std::string("\xF0\x80\x80\x80", 4)),
           "Reject 4-byte overlong sequence");

    Expect(!Utf8::Validate(std::string("\xED\xA0\x80", 3)),
           "Reject UTF-8 surrogate");

    Expect(!Utf8::Validate(std::string("\xF4\x90\x80\x80", 4)),
           "Reject above U+10FFFF");

    Expect(!Utf8::Validate(std::string("\xE0\xAE", 2)),
           "Reject truncated 3-byte sequence");

    Expect(!Utf8::Validate(std::string("\xF0\x9F\x98", 3)),
           "Reject truncated 4-byte sequence");

    auto encode_ascii = Utf8::Encode(0x41);
    Expect(encode_ascii.Ok(), "Encode ASCII");
    Expect(encode_ascii.Value() == "A", "Encoded ASCII bytes");

    auto encode_tamil = Utf8::Encode(0x0B85);
    Expect(encode_tamil.Ok(), "Encode Tamil");
    Expect(encode_tamil.Value() == "\xE0\xAE\x85",
           "Encoded Tamil bytes");

    auto encode_emoji = Utf8::Encode(0x1F600);
    Expect(encode_emoji.Ok(), "Encode emoji");
    Expect(encode_emoji.Value() == "\xF0\x9F\x98\x80",
           "Encoded emoji bytes");

    auto invalid_surrogate = Utf8::Encode(0xD800);
    Expect(invalid_surrogate.Failed(), "Reject encoding surrogate");

    auto invalid_high = Utf8::Encode(0x110000);
    Expect(invalid_high.Failed(), "Reject encoding above Unicode maximum");

    auto roundtrip = Utf8::Encode(emoji.Value().codepoint);
    Expect(roundtrip.Ok(), "Emoji round-trip encode");
    Expect(roundtrip.Value() == "\xF0\x9F\x98\x80",
           "Emoji round-trip integrity");

    return Summary();
}
