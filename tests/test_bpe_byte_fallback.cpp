#include <iostream>
#include <string>
#include <vector>

#include "bpe/byte_fallback.hpp"
#include "core/types.hpp"

using namespace qualix;
using namespace qualix::bpe;

namespace
{

usize tests_run = 0;
usize tests_passed = 0;

void Expect(
    bool condition,
    const char* name
)
{
    ++tests_run;

    if (condition)
    {
        ++tests_passed;

        std::cout
            << "[PASS] "
            << name
            << '\n';
    }
    else
    {
        std::cout
            << "[FAIL] "
            << name
            << '\n';
    }
}

bool RoundTrip(
    const std::string& text
)
{
    const auto encoded =
        ByteFallback::Encode(text);

    const auto decoded =
        ByteFallback::Decode(
            encoded
        );

    return decoded == text;
}

} // namespace

int main()
{
    /*
     * All 256 byte values must have unique,
     * deterministic representations.
     */

    std::vector<std::string> symbols;

    symbols.reserve(
        ByteFallback::ByteCount
    );

    bool all_parse = true;

    for (usize i = 0;
         i < ByteFallback::ByteCount;
         ++i)
    {
        const u8 expected =
            static_cast<u8>(i);

        const std::string symbol =
            ByteFallback::Symbol(
                expected
            );

        u8 restored = 0;

        if (!ByteFallback::ParseSymbol(
                symbol,
                restored) ||
            restored != expected)
        {
            all_parse = false;
            break;
        }

        symbols.push_back(
            symbol
        );
    }

    Expect(
        symbols.size() == 256,
        "All 256 byte symbols generated"
    );

    Expect(
        all_parse,
        "All byte symbols parse exactly"
    );

    Expect(
        ByteFallback::Symbol(0x00) ==
            "<0x00>",
        "Byte zero representation"
    );

    Expect(
        ByteFallback::Symbol(0xFF) ==
            "<0xFF>",
        "Byte 255 representation"
    );

    /*
     * Invalid fallback symbols.
     */

    {
        u8 byte = 0;

        Expect(
            !ByteFallback::ParseSymbol(
                "<UNK>",
                byte
            ),
            "Non-byte symbol rejected"
        );

        Expect(
            !ByteFallback::ParseSymbol(
                "<0xGG>",
                byte
            ),
            "Invalid hex rejected"
        );

        Expect(
            !ByteFallback::ParseSymbol(
                "0x41",
                byte
            ),
            "Malformed symbol rejected"
        );
    }

    /*
     * ASCII.
     */

    Expect(
        RoundTrip(
            "hello world"
        ),
        "ASCII exact round trip"
    );

    /*
     * Tanglish.
     */

    Expect(
        RoundTrip(
            "Enakku Tamil pesa theriyum"
        ),
        "Tanglish exact round trip"
    );

    /*
     * Tamil.
     */

    Expect(
        RoundTrip(
            "தமிழ் ஒரு அழகான மொழி"
        ),
        "Tamil exact round trip"
    );

    /*
     * Korean — useful example of a script that
     * may never have appeared during training.
     */

    Expect(
        RoundTrip(
            "안녕하세요"
        ),
        "Korean unknown-script round trip"
    );

    /*
     * Japanese.
     */

    Expect(
        RoundTrip(
            "こんにちは世界"
        ),
        "Japanese unknown-script round trip"
    );

    /*
     * Arabic.
     */

    Expect(
        RoundTrip(
            "مرحبا بالعالم"
        ),
        "Arabic unknown-script round trip"
    );

    /*
     * Emoji and multi-byte Unicode sequences.
     */

    Expect(
        RoundTrip(
            "Hello ❤️🔥🚀"
        ),
        "Emoji exact round trip"
    );

    /*
     * Mixed multilingual input.
     */

    Expect(
        RoundTrip(
            "Hello தமிழ் 안녕 日本語 مرحبا ❤️"
        ),
        "Multilingual exact round trip"
    );

    /*
     * Every raw byte value must survive.
     *
     * This verifies the fallback primitive itself,
     * independent of UTF-8 validity.
     */

    std::string all_bytes;

    all_bytes.reserve(256);

    for (usize i = 0;
         i < 256;
         ++i)
    {
        all_bytes.push_back(
            static_cast<char>(
                static_cast<u8>(i)
            )
        );
    }

    Expect(
        RoundTrip(all_bytes),
        "All 256 raw bytes exact round trip"
    );

    /*
     * Empty input.
     */

    Expect(
        RoundTrip(""),
        "Empty input round trip"
    );

    std::cout
        << '\n'
        << tests_passed
        << "/"
        << tests_run
        << " tests passed\n";

    return
        tests_passed == tests_run
            ? 0
            : 1;
}
