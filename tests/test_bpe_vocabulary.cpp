#include <iostream>
#include <string_view>

#include "bpe/vocabulary.hpp"

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
    Vocabulary vocabulary;

    Expect(
        vocabulary.Size() == 0,
        "Vocabulary initially empty"
    );

    Expect(
        vocabulary.NextId() == 1,
        "ID zero permanently reserved"
    );

    const SymbolId a =
        vocabulary.Add("a");

    const SymbolId b =
        vocabulary.Add("b");

    Expect(
        a == 1,
        "First symbol receives ID 1"
    );

    Expect(
        b == 2,
        "Second symbol receives ID 2"
    );

    Expect(
        vocabulary.Size() == 2,
        "Vocabulary size correct"
    );

    Expect(
        vocabulary.NextId() == 3,
        "Next ID appended"
    );

    const SymbolId duplicate_a =
        vocabulary.Add("a");

    Expect(
        duplicate_a == a,
        "Duplicate symbol preserves ID"
    );

    Expect(
        vocabulary.Size() == 2,
        "Duplicate does not grow vocabulary"
    );

    const auto a_id =
        vocabulary.Find("a");

    Expect(
        a_id.has_value() &&
        *a_id == 1,
        "Symbol to ID lookup"
    );

    const auto b_text =
        vocabulary.Find(2);

    Expect(
        b_text.has_value() &&
        *b_text == "b",
        "ID to symbol lookup"
    );

    Expect(
        vocabulary.Contains("a"),
        "Contains existing symbol"
    );

    Expect(
        vocabulary.Contains(1),
        "Contains existing ID"
    );

    Expect(
        !vocabulary.Contains("missing"),
        "Missing symbol rejected"
    );

    Expect(
        !vocabulary.Contains(999),
        "Missing ID rejected"
    );

    Expect(
        !vocabulary.Contains(
            InvalidSymbolId
        ),
        "Invalid ID rejected"
    );

    const SymbolId empty =
        vocabulary.Add("");

    Expect(
        empty == InvalidSymbolId,
        "Empty symbol rejected"
    );

    /*
     * Unicode strings must remain byte-exact.
     */

    const SymbolId tamil =
        vocabulary.Add("தமிழ்");

    const auto tamil_text =
        vocabulary.Find(tamil);

    Expect(
        tamil_text.has_value() &&
        *tamil_text == "தமிழ்",
        "Tamil symbol round trip"
    );

    const SymbolId emoji =
        vocabulary.Add("❤️");

    const auto emoji_text =
        vocabulary.Find(emoji);

    Expect(
        emoji_text.has_value() &&
        *emoji_text == "❤️",
        "Emoji symbol round trip"
    );

    /*
     * IDs must only append.
     */

    Expect(
        tamil > b &&
        emoji > tamil,
        "IDs monotonically appended"
    );

    const SymbolId merged =
        vocabulary.Add("ab");

    Expect(
        merged == vocabulary.Size(),
        "Merged symbol appended"
    );

    Expect(
        vocabulary.Find("a").value() == a &&
        vocabulary.Find("b").value() == b,
        "Old IDs preserved after append"
    );


    {
        Vocabulary vocabulary;

        const SymbolId t =
            vocabulary.Add("t");

        const SymbolId h =
            vocabulary.Add("h");

        const SymbolId th =
            vocabulary.AddMerged(
                t,
                h
            );

        Expect(
            th != InvalidSymbolId,
            "Merged symbol created"
        );

        const auto value =
            vocabulary.Find(th);

        Expect(
            value.has_value() &&
            *value == "th",
            "Merged symbol text correct"
        );

        Expect(
            vocabulary.Size() == 3,
            "Merged symbol extends vocabulary"
        );
    }

    {
        Vocabulary vocabulary;

        const SymbolId a =
            vocabulary.Add("a");

        const SymbolId b =
            vocabulary.Add("b");

        const SymbolId ab1 =
            vocabulary.AddMerged(
                a,
                b
            );

        const SymbolId ab2 =
            vocabulary.AddMerged(
                a,
                b
            );

        Expect(
            ab1 == ab2,
            "Duplicate merged symbol reuses ID"
        );

        Expect(
            vocabulary.Size() == 3,
            "Duplicate merge does not grow vocabulary"
        );
    }

    {
        Vocabulary vocabulary;

        const SymbolId a =
            vocabulary.Add("a");

        const SymbolId invalid =
            vocabulary.AddMerged(
                a,
                999999
            );

        Expect(
            invalid == InvalidSymbolId,
            "Invalid right symbol rejected"
        );
    }

    {
        Vocabulary vocabulary;

        const SymbolId b =
            vocabulary.Add("b");

        const SymbolId invalid =
            vocabulary.AddMerged(
                999999,
                b
            );

        Expect(
            invalid == InvalidSymbolId,
            "Invalid left symbol rejected"
        );
    }

    {
        Vocabulary vocabulary;

        const SymbolId a =
            vocabulary.Add("a");

        const SymbolId b =
            vocabulary.Add("b");

        const SymbolId ab =
            vocabulary.AddMerged(
                a,
                b
            );

        const SymbolId c =
            vocabulary.Add("c");

        const SymbolId abc =
            vocabulary.AddMerged(
                ab,
                c
            );

        const auto value =
            vocabulary.Find(abc);

        Expect(
            value.has_value() &&
            *value == "abc",
            "Recursive merged symbol correct"
        );
    }

    const usize failed =
        tests_run - tests_passed;

    std::cout
        << "\n================================\n"
        << "Tests Run    : "
        << tests_run
        << '\n'
        << "Tests Passed : "
        << tests_passed
        << '\n'
        << "Tests Failed : "
        << failed
        << '\n'
        << "================================\n";

    
    // QUALIX_EXACT_ID_IMPORT_TESTS
    // ---------------------------------------------------------
    // Exact-ID vocabulary import
    // ---------------------------------------------------------

    {
        Vocabulary vocabulary;

        Expect(
            vocabulary.AddWithId(
                1,
                "a"
            ),
            "Exact ID import accepts first symbol"
        );

        Expect(
            vocabulary.AddWithId(
                2,
                "b"
            ),
            "Exact ID import accepts sequential symbol"
        );

        const auto a =
            vocabulary.Find(
                static_cast<SymbolId>(1)
            );

        const auto b =
            vocabulary.Find(
                static_cast<SymbolId>(2)
            );

        Expect(
            a.has_value() &&
            *a == "a",
            "Exact ID 1 preserved"
        );

        Expect(
            b.has_value() &&
            *b == "b",
            "Exact ID 2 preserved"
        );

        Expect(
            vocabulary.Find("a") ==
                std::optional<SymbolId>{1},
            "Reverse lookup preserves imported ID"
        );

        Expect(
            vocabulary.Find("b") ==
                std::optional<SymbolId>{2},
            "Reverse lookup preserves second imported ID"
        );
    }

    {
        Vocabulary vocabulary;

        Expect(
            !vocabulary.AddWithId(
                InvalidSymbolId,
                "invalid"
            ),
            "Reserved ID zero rejected"
        );

        Expect(
            !vocabulary.AddWithId(
                1,
                ""
            ),
            "Empty imported symbol rejected"
        );

        Expect(
            vocabulary.Size() == 0,
            "Rejected imports do not mutate vocabulary"
        );
    }

    {
        Vocabulary vocabulary;

        Expect(
            !vocabulary.AddWithId(
                2,
                "gap"
            ),
            "Non-sequential exact ID rejected"
        );

        Expect(
            vocabulary.Size() == 0,
            "ID gap rejection preserves vocabulary"
        );
    }

    {
        Vocabulary vocabulary;

        Expect(
            vocabulary.AddWithId(
                1,
                "x"
            ),
            "Initial exact mapping accepted"
        );

        Expect(
            vocabulary.AddWithId(
                1,
                "x"
            ),
            "Identical exact mapping is idempotent"
        );

        Expect(
            !vocabulary.AddWithId(
                1,
                "y"
            ),
            "Conflicting text for occupied ID rejected"
        );

        Expect(
            !vocabulary.AddWithId(
                2,
                "x"
            ),
            "Duplicate symbol with different ID rejected"
        );

        Expect(
            vocabulary.Size() == 1,
            "Conflicting imports do not change vocabulary size"
        );
    }

    {
        Vocabulary vocabulary;

        Expect(
            vocabulary.AddWithId(
                1,
                "a"
            ),
            "Imported ID before normal Add"
        );

        const SymbolId b =
            vocabulary.Add(
                "b"
            );

        Expect(
            b == 2,
            "Normal Add continues after imported ID"
        );

        Expect(
            vocabulary.NextId() == 3,
            "NextId remains stable after import"
        );
    }

    {
        Vocabulary vocabulary;

        Expect(
            vocabulary.AddWithId(
                1,
                "a"
            ),
            "Merged import base left"
        );

        Expect(
            vocabulary.AddWithId(
                2,
                "b"
            ),
            "Merged import base right"
        );

        const SymbolId merged =
            vocabulary.AddMerged(
                1,
                2
            );

        Expect(
            merged == 3,
            "Merged symbol follows imported IDs"
        );

        const auto value =
            vocabulary.Find(
                merged
            );

        Expect(
            value.has_value() &&
            *value == "ab",
            "Merged text correct after exact import"
        );
    }

return failed == 0 ? 0 : 1;
}
