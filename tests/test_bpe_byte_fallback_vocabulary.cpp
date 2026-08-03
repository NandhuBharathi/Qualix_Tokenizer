#include <iostream>
#include <string>

#include "bpe/byte_fallback.hpp"
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

} // namespace

int main()
{
    /*
     * Fallback is opt-in.
     */

    {
        Vocabulary vocabulary;

        Expect(
            !vocabulary.HasByteFallback(),
            "Fresh vocabulary has no fallback"
        );

        Expect(
            !vocabulary.FindByte(
                static_cast<u8>('A')
            ).has_value(),
            "Byte lookup unavailable before registration"
        );
    }

    /*
     * Existing IDs must remain unchanged.
     */

    Vocabulary vocabulary;

    const SymbolId hello =
        vocabulary.Add("hello");

    const SymbolId tamil =
        vocabulary.Add("தமிழ்");

    const usize old_size =
        vocabulary.Size();

    const SymbolId old_next =
        vocabulary.NextId();

    Expect(
        hello == 1 &&
        tamil == 2,
        "Existing IDs established"
    );

    Expect(
        vocabulary.EnsureByteFallback(),
        "Fallback registration succeeds"
    );

    Expect(
        vocabulary.HasByteFallback(),
        "Fallback alphabet detected"
    );

    Expect(
        vocabulary.Find("hello").value() ==
            hello &&
        vocabulary.Find("தமிழ்").value() ==
            tamil,
        "Existing IDs preserved"
    );

    Expect(
        vocabulary.Size() ==
            old_size +
            ByteFallback::ByteCount,
        "Exactly 256 symbols appended"
    );

    /*
     * Byte IDs must form one deterministic,
     * contiguous block.
     */

    bool contiguous = true;

    for (usize i = 0;
         i < ByteFallback::ByteCount;
         ++i)
    {
        const auto id =
            vocabulary.FindByte(
                static_cast<u8>(i)
            );

        if (!id.has_value() ||
            *id !=
                static_cast<SymbolId>(
                    old_next + i
                ))
        {
            contiguous = false;
            break;
        }
    }

    Expect(
        contiguous,
        "Byte fallback IDs contiguous"
    );

    const auto zero =
        vocabulary.FindByte(0x00);

    const auto ascii_a =
        vocabulary.FindByte(
            static_cast<u8>('A')
        );

    const auto ff =
        vocabulary.FindByte(0xFF);

    Expect(
        zero.has_value() &&
        *zero == old_next,
        "Byte zero receives fallback base ID"
    );

    Expect(
        ascii_a.has_value() &&
        *ascii_a ==
            static_cast<SymbolId>(
                old_next + 0x41
            ),
        "ASCII byte ID deterministic"
    );

    Expect(
        ff.has_value() &&
        *ff ==
            static_cast<SymbolId>(
                old_next + 0xFF
            ),
        "Byte 255 ID deterministic"
    );

    /*
     * Reverse vocabulary lookup must preserve the
     * exact fallback textual representation.
     */

    bool reverse_exact = true;

    for (usize i = 0;
         i < ByteFallback::ByteCount;
         ++i)
    {
        const auto id =
            vocabulary.FindByte(
                static_cast<u8>(i)
            );

        if (!id.has_value())
        {
            reverse_exact = false;
            break;
        }

        const auto symbol =
            vocabulary.Find(*id);

        if (!symbol.has_value() ||
            *symbol !=
                ByteFallback::Symbol(
                    static_cast<u8>(i)
                ))
        {
            reverse_exact = false;
            break;
        }
    }

    Expect(
        reverse_exact,
        "Fallback ID to symbol mapping exact"
    );

    /*
     * Registration must be idempotent.
     */

    const usize size_before_second =
        vocabulary.Size();

    const SymbolId next_before_second =
        vocabulary.NextId();

    Expect(
        vocabulary.EnsureByteFallback(),
        "Second fallback registration succeeds"
    );

    Expect(
        vocabulary.Size() ==
            size_before_second &&
        vocabulary.NextId() ==
            next_before_second,
        "Fallback registration idempotent"
    );

    /*
     * New ordinary vocabulary symbols continue
     * after the fallback block.
     */

    const SymbolId after =
        vocabulary.Add(
            "after-fallback"
        );

    Expect(
        after ==
            next_before_second,
        "Normal vocabulary append continues"
    );

    Expect(
        vocabulary.HasByteFallback(),
        "Fallback remains valid after append"
    );

    /*
     * Collision safety.
     *
     * Ordinary text that happens to equal the
     * fallback syntax must not silently be treated
     * as a byte token.
     */

    {
        Vocabulary collision;

        const SymbolId existing =
            collision.Add("<0x41>");

        const usize before =
            collision.Size();

        Expect(
            existing != InvalidSymbolId,
            "Collision symbol inserted normally"
        );

        Expect(
            !collision.EnsureByteFallback(),
            "Partial fallback namespace collision rejected"
        );

        Expect(
            collision.Size() == before,
            "Collision rejection does not grow vocabulary"
        );

        Expect(
            !collision.HasByteFallback(),
            "Collision does not enable fallback"
        );
    }

    /*
     * Empty vocabulary deterministic layout.
     */

    {
        Vocabulary empty;

        Expect(
            empty.EnsureByteFallback(),
            "Empty vocabulary fallback registration"
        );

        const auto first =
            empty.FindByte(0x00);

        const auto last =
            empty.FindByte(0xFF);

        Expect(
            first.has_value() &&
            *first == 1,
            "Empty vocabulary fallback starts at ID 1"
        );

        Expect(
            last.has_value() &&
            *last == 256,
            "Empty vocabulary fallback ends at ID 256"
        );

        Expect(
            empty.Size() == 256 &&
            empty.NextId() == 257,
            "Empty vocabulary fallback size exact"
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
