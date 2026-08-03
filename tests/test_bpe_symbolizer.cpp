#include <iostream>
#include <string>
#include <vector>

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

} // namespace

int main()
{
    {
        Vocabulary vocabulary;

        const auto result =
            Symbolizer::Symbolize(
                "",
                vocabulary
            );

        Expect(
            result.Ok(),
            "Empty input succeeds"
        );

        Expect(
            result.Value().empty(),
            "Empty input has no symbols"
        );

        Expect(
            vocabulary.Size() == 0,
            "Empty input does not grow vocabulary"
        );
    }

    {
        Vocabulary vocabulary;

        const auto result =
            Symbolizer::Symbolize(
                "hello",
                vocabulary
            );

        Expect(
            result.Ok(),
            "ASCII symbolization succeeds"
        );

        Expect(
            result.Value().size() == 5,
            "ASCII grapheme count correct"
        );

        const auto h =
            vocabulary.Find("h");

        const auto e =
            vocabulary.Find("e");

        const auto l =
            vocabulary.Find("l");

        const auto o =
            vocabulary.Find("o");

        Expect(
            h.has_value() &&
            e.has_value() &&
            l.has_value() &&
            o.has_value(),
            "ASCII graphemes added"
        );

        Expect(
            result.Value().size() == 5 &&
            result.Value()[2] ==
                result.Value()[3],
            "Repeated grapheme reuses ID"
        );

        Expect(
            vocabulary.Size() == 4,
            "Repeated symbol does not duplicate vocabulary"
        );
    }

    {
        Vocabulary vocabulary;

        const auto result =
            Symbolizer::Symbolize(
                "தமிழ்",
                vocabulary
            );

        Expect(
            result.Ok(),
            "Tamil symbolization succeeds"
        );

        /*
         * Qualix GraphemeSegmenter currently
         * segments:
         *
         * த
         * மி
         * ழ்
         */
        Expect(
            result.Value().size() == 3,
            "Tamil uses grapheme symbols"
        );

        Expect(
            vocabulary.Contains("த"),
            "Tamil first grapheme stored"
        );

        Expect(
            vocabulary.Contains("மி"),
            "Tamil combining grapheme stored"
        );

        Expect(
            vocabulary.Contains("ழ்"),
            "Tamil final grapheme stored"
        );
    }

    {
        Vocabulary vocabulary;

        const auto result =
            Symbolizer::Symbolize(
                "❤️",
                vocabulary
            );

        Expect(
            result.Ok(),
            "Emoji symbolization succeeds"
        );

        Expect(
            result.Value().size() == 1,
            "Emoji sequence remains one grapheme"
        );

        Expect(
            vocabulary.Contains("❤️"),
            "Emoji stored byte-exact"
        );
    }

    {
        Vocabulary vocabulary;

        const auto result =
            Symbolizer::Symbolize(
                "👍🏽",
                vocabulary
            );

        Expect(
            result.Ok(),
            "Emoji modifier symbolization succeeds"
        );

        Expect(
            result.Value().size() == 1,
            "Emoji modifier remains one grapheme"
        );

        Expect(
            vocabulary.Contains("👍🏽"),
            "Emoji modifier stored as one symbol"
        );
    }

    {
        Vocabulary vocabulary;

        const auto first =
            Symbolizer::Symbolize(
                "hello",
                vocabulary
            );

        const usize size_after_first =
            vocabulary.Size();

        const auto second =
            Symbolizer::Symbolize(
                "hello",
                vocabulary
            );

        Expect(
            first.Ok() &&
            second.Ok(),
            "Repeated symbolization succeeds"
        );

        Expect(
            first.Value() ==
            second.Value(),
            "Repeated text preserves IDs"
        );

        Expect(
            vocabulary.Size() ==
                size_after_first,
            "Repeated text does not grow vocabulary"
        );
    }

    {
        Vocabulary vocabulary;

        const auto first =
            Symbolizer::Symbolize(
                "ab",
                vocabulary
            );

        const auto second =
            Symbolizer::Symbolize(
                "ac",
                vocabulary
            );

        Expect(
            first.Ok() &&
            second.Ok(),
            "Incremental vocabulary succeeds"
        );

        const auto a =
            vocabulary.Find("a");

        Expect(
            a.has_value() &&
            first.Value()[0] == *a &&
            second.Value()[0] == *a,
            "Existing ID preserved across inputs"
        );

        Expect(
            vocabulary.Size() == 3,
            "Only unseen grapheme appended"
        );
    }

    {
        /*
         * e + COMBINING ACUTE ACCENT
         *
         * This is intentionally decomposed.
         * Grapheme segmentation must keep the
         * base and combining mark together.
         */
        Vocabulary vocabulary;

        const std::string decomposed =
            "e\xCC\x81";

        const auto result =
            Symbolizer::Symbolize(
                decomposed,
                vocabulary
            );

        Expect(
            result.Ok(),
            "Combining sequence succeeds"
        );

        Expect(
            result.Value().size() == 1,
            "Combining sequence is one grapheme"
        );

        Expect(
            vocabulary.Size() == 1,
            "Combining sequence one vocabulary symbol"
        );
    }

    {
        /*
         * Invalid UTF-8 must propagate the
         * GraphemeSegmenter failure.
         */
        Vocabulary vocabulary;

        const std::string invalid{
            static_cast<char>(0xFF)
        };

        const auto result =
            Symbolizer::Symbolize(
                invalid,
                vocabulary
            );

        Expect(
            result.Failed(),
            "Invalid UTF-8 propagates failure"
        );

        Expect(
            vocabulary.Size() == 0,
            "Invalid UTF-8 does not modify vocabulary"
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

    return failed == 0 ? 0 : 1;
}
