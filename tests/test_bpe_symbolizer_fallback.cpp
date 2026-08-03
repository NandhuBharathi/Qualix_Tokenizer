#include <iostream>
#include <string>
#include <vector>

#include "bpe/byte_fallback.hpp"
#include "bpe/symbolizer.hpp"

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

bool MatchesBytes(
    const Vocabulary& vocabulary,
    const std::vector<SymbolId>& ids,
    std::string_view text
)
{
    if (ids.size() != text.size())
        return false;

    for (usize i = 0;
         i < text.size();
         ++i)
    {
        const auto expected =
            vocabulary.FindByte(
                static_cast<u8>(
                    static_cast<
                        unsigned char
                    >(text[i])
                )
            );

        if (!expected.has_value() ||
            ids[i] != *expected)
        {
            return false;
        }
    }

    return true;
}

} // namespace

int main()
{
    /*
     * Missing fallback alphabet must fail rather
     * than silently mutate the vocabulary.
     */
    {
        Vocabulary vocabulary;

        const usize before =
            vocabulary.Size();

        const auto result =
            Symbolizer::
                SymbolizeWithFallback(
                    "hello",
                    vocabulary
                );

        Expect(
            result.Failed(),
            "Missing byte fallback rejected"
        );

        Expect(
            vocabulary.Size() == before,
            "Failure does not grow vocabulary"
        );
    }

    /*
     * Exact learned symbols always win over
     * byte fallback.
     */
    {
        Vocabulary vocabulary;

        const SymbolId h =
            vocabulary.Add("h");

        const SymbolId i =
            vocabulary.Add("i");

        const bool fallback =
            vocabulary.EnsureByteFallback();

        const usize before =
            vocabulary.Size();

        const auto result =
            Symbolizer::
                SymbolizeWithFallback(
                    "hi",
                    vocabulary
                );

        Expect(
            fallback &&
            result.Ok(),
            "Known ASCII inference succeeds"
        );

        Expect(
            result.Ok() &&
            result.Value().size() == 2 &&
            result.Value()[0] == h &&
            result.Value()[1] == i,
            "Known graphemes preserve learned IDs"
        );

        Expect(
            vocabulary.Size() == before,
            "Known inference does not grow vocabulary"
        );
    }

    /*
     * Unknown ASCII grapheme becomes one
     * byte-fallback ID.
     */
    {
        Vocabulary vocabulary;

        const SymbolId known =
            vocabulary.Add("a");

        const bool fallback =
            vocabulary.EnsureByteFallback();

        const usize before =
            vocabulary.Size();

        const auto result =
            Symbolizer::
                SymbolizeWithFallback(
                    "az",
                    vocabulary
                );

        const auto z =
            vocabulary.FindByte(
                static_cast<u8>('z')
            );

        Expect(
            fallback &&
            result.Ok(),
            "Mixed known and OOV ASCII succeeds"
        );

        Expect(
            result.Ok() &&
            z.has_value() &&
            result.Value().size() == 2 &&
            result.Value()[0] == known &&
            result.Value()[1] == *z,
            "OOV ASCII uses byte fallback"
        );

        Expect(
            vocabulary.Size() == before &&
            !vocabulary.Contains("z"),
            "OOV ASCII does not enter vocabulary"
        );
    }

    /*
     * Unknown Tamil grapheme must expand to its
     * exact UTF-8 bytes.
     */
    {
        Vocabulary vocabulary;

        const bool fallback =
            vocabulary.EnsureByteFallback();

        const usize before =
            vocabulary.Size();

        const std::string text =
            "ழ";

        const auto result =
            Symbolizer::
                SymbolizeWithFallback(
                    text,
                    vocabulary
                );

        Expect(
            fallback &&
            result.Ok(),
            "Unknown Tamil fallback succeeds"
        );

        Expect(
            result.Ok() &&
            MatchesBytes(
                vocabulary,
                result.Value(),
                text
            ),
            "Unknown Tamil preserved as UTF-8 bytes"
        );

        Expect(
            vocabulary.Size() == before &&
            !vocabulary.Contains(text),
            "Tamil OOV does not grow vocabulary"
        );
    }

    /*
     * Unknown emoji is also byte-lossless.
     */
    {
        Vocabulary vocabulary;

        const bool fallback =
            vocabulary.EnsureByteFallback();

        const usize before =
            vocabulary.Size();

        const std::string text =
            "🧠";

        const auto result =
            Symbolizer::
                SymbolizeWithFallback(
                    text,
                    vocabulary
                );

        Expect(
            fallback &&
            result.Ok(),
            "Unknown emoji fallback succeeds"
        );

        Expect(
            result.Ok() &&
            MatchesBytes(
                vocabulary,
                result.Value(),
                text
            ),
            "Unknown emoji preserved as UTF-8 bytes"
        );

        Expect(
            vocabulary.Size() == before,
            "Emoji OOV does not grow vocabulary"
        );
    }

    /*
     * Tanglish-style input:
     *
     * learned fragments retain learned IDs,
     * unseen characters fall back independently.
     */
    {
        Vocabulary vocabulary;

        const SymbolId n =
            vocabulary.Add("n");

        const SymbolId a =
            vocabulary.Add("a");

        const SymbolId m =
            vocabulary.Add("m");

        const bool fallback =
            vocabulary.EnsureByteFallback();

        const usize before =
            vocabulary.Size();

        const auto result =
            Symbolizer::
                SymbolizeWithFallback(
                    "namma!",
                    vocabulary
                );

        const auto bang =
            vocabulary.FindByte(
                static_cast<u8>('!')
            );

        Expect(
            fallback &&
            result.Ok(),
            "Tanglish-style mixed inference succeeds"
        );

        Expect(
            result.Ok() &&
            bang.has_value() &&
            result.Value().size() == 6 &&
            result.Value()[0] == n &&
            result.Value()[1] == a &&
            result.Value()[2] == m &&
            result.Value()[3] == m &&
            result.Value()[4] == a &&
            result.Value()[5] == *bang,
            "Known and fallback IDs mix correctly"
        );

        Expect(
            vocabulary.Size() == before,
            "Mixed inference keeps vocabulary frozen"
        );
    }

    /*
     * Existing training API must retain its
     * original vocabulary-building semantics.
     */
    {
        Vocabulary vocabulary;

        const auto result =
            Symbolizer::Symbolize(
                "abc",
                vocabulary
            );

        Expect(
            result.Ok() &&
            vocabulary.Contains("a") &&
            vocabulary.Contains("b") &&
            vocabulary.Contains("c"),
            "Training symbolizer behavior preserved"
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
