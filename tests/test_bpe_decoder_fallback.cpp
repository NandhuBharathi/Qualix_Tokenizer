#include <iostream>
#include <string>
#include <vector>

#include "bpe/byte_fallback.hpp"
#include "bpe/decoder.hpp"
#include "bpe/vocabulary.hpp"
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

std::vector<SymbolId>
FallbackIds(
    std::string_view text,
    const Vocabulary& vocabulary
)
{
    std::vector<SymbolId> ids;

    ids.reserve(text.size());

    for (const unsigned char value : text)
    {
        const auto id =
            vocabulary.FindByte(
                static_cast<u8>(value)
            );

        if (!id.has_value())
            return {};

        ids.push_back(*id);
    }

    return ids;
}

} // namespace

int main()
{
    /*
     * Complete fallback alphabet.
     */
    Vocabulary vocabulary;

    const bool fallback_ready =
        vocabulary.EnsureByteFallback();

    Expect(
        fallback_ready,
        "Byte fallback vocabulary registration succeeds"
    );

    Expect(
        vocabulary.HasByteFallback(),
        "Vocabulary reports complete byte fallback"
    );

    if (!fallback_ready)
        return 1;

    /*
     * ASCII byte fallback.
     */
    {
        const std::string input =
            "hello";

        const auto ids =
            FallbackIds(
                input,
                vocabulary
            );

        auto decoded =
            BpeDecoder::Decode(
                ids,
                vocabulary
            );

        Expect(
            decoded.Ok() &&
            decoded.Value() == input,
            "ASCII fallback bytes decode exactly"
        );
    }

    /*
     * Tamil UTF-8 byte fallback.
     */
    {
        const std::string input =
            "தமிழ்";

        const auto ids =
            FallbackIds(
                input,
                vocabulary
            );

        Expect(
            ids.size() == input.size(),
            "Tamil fallback produces one ID per UTF-8 byte"
        );

        auto decoded =
            BpeDecoder::Decode(
                ids,
                vocabulary
            );

        Expect(
            decoded.Ok() &&
            decoded.Value() == input,
            "Tamil fallback reconstructs exact UTF-8"
        );
    }

    /*
     * Emoji UTF-8 byte fallback.
     */
    {
        const std::string input =
            "🧠";

        const auto ids =
            FallbackIds(
                input,
                vocabulary
            );

        auto decoded =
            BpeDecoder::Decode(
                ids,
                vocabulary
            );

        Expect(
            decoded.Ok() &&
            decoded.Value() == input,
            "Emoji fallback reconstructs exact UTF-8"
        );
    }

    /*
     * Arbitrary multilingual text.
     */
    {
        const std::string input =
            "தமிழ் English 日本語 한국어 🧠❤️";

        const auto ids =
            FallbackIds(
                input,
                vocabulary
            );

        auto decoded =
            BpeDecoder::Decode(
                ids,
                vocabulary
            );

        Expect(
            decoded.Ok() &&
            decoded.Value() == input,
            "Multilingual fallback round trip exact"
        );
    }

    /*
     * Mixed learned symbol + fallback bytes.
     *
     * This is the important real inference case:
     *
     * learned BPE token
     *       +
     * unknown UTF-8 bytes
     *       +
     * learned BPE token
     */
    {
        Vocabulary mixed;

        const SymbolId hello =
            mixed.Add("hello ");

        const bool ready =
            mixed.EnsureByteFallback();

        const SymbolId suffix =
            mixed.Add(" world");

        Expect(
            ready,
            "Mixed vocabulary fallback registration succeeds"
        );

        const std::string unknown =
            "🧠";

        std::vector<SymbolId> ids;

        ids.push_back(hello);

        const auto fallback =
            FallbackIds(
                unknown,
                mixed
            );

        ids.insert(
            ids.end(),
            fallback.begin(),
            fallback.end()
        );

        ids.push_back(suffix);

        auto decoded =
            BpeDecoder::Decode(
                ids,
                mixed
            );

        Expect(
            decoded.Ok() &&
            decoded.Value() ==
                "hello 🧠 world",
            "Learned and fallback symbols decode together"
        );
    }

    /*
     * Every possible byte value must survive
     * fallback decoding exactly.
     */
    {
        std::string input;

        input.reserve(
            ByteFallback::ByteCount
        );

        for (usize i = 0;
             i < ByteFallback::ByteCount;
             ++i)
        {
            input.push_back(
                static_cast<char>(
                    static_cast<u8>(i)
                )
            );
        }

        const auto ids =
            FallbackIds(
                input,
                vocabulary
            );

        auto decoded =
            BpeDecoder::Decode(
                ids,
                vocabulary
            );

        Expect(
            decoded.Ok() &&
            decoded.Value() == input,
            "All 256 byte values decode exactly"
        );
    }

    /*
     * Existing normal symbol behavior must remain
     * unchanged.
     */
    {
        Vocabulary normal;

        const SymbolId tamil =
            normal.Add("தமிழ்");

        const SymbolId space =
            normal.Add(" ");

        const SymbolId english =
            normal.Add("hello");

        const std::vector<SymbolId> ids{
            tamil,
            space,
            english
        };

        auto decoded =
            BpeDecoder::Decode(
                ids,
                normal
            );

        Expect(
            decoded.Ok() &&
            decoded.Value() ==
                "தமிழ் hello",
            "Normal learned symbols remain unchanged"
        );
    }

    /*
     * Invalid IDs must still fail.
     */
    {
        const std::vector<SymbolId> ids{
            InvalidSymbolId
        };

        auto decoded =
            BpeDecoder::Decode(
                ids,
                vocabulary
            );

        Expect(
            decoded.Failed(),
            "InvalidSymbolId still rejected"
        );
    }

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
