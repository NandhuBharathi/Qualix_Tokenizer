#include <iostream>
#include <string>
#include <vector>

#include "bpe/model.hpp"

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
     * Fresh model.
     */
    {
        BpeModel model;

        Expect(
            model.Empty(),
            "Fresh model empty"
        );

        Expect(
            model.VocabularySize() == 0,
            "Fresh vocabulary empty"
        );

        Expect(
            model.RuleCount() == 0,
            "Fresh rule set empty"
        );
    }

    /*
     * Vocabulary ownership.
     *
     * Direct vocabulary mutation remains
     * available for training/model construction.
     */
    {
        BpeModel model;

        auto& vocabulary =
            model.GetVocabulary();

        const SymbolId a =
            vocabulary.Add("a");

        const SymbolId b =
            vocabulary.Add("b");

        Expect(
            a != InvalidSymbolId,
            "Model vocabulary adds first symbol"
        );

        Expect(
            b != InvalidSymbolId,
            "Model vocabulary adds second symbol"
        );

        Expect(
            model.VocabularySize() == 2,
            "Model reports vocabulary size"
        );

        Expect(
            !model.Empty(),
            "Model not empty after vocabulary insert"
        );
    }

    /*
     * Rule ownership.
     */
    {
        BpeModel model;

        std::vector<MergeRule> rules{
            MergeRule{
                Pair{1, 2},
                3,
                0
            },
            MergeRule{
                Pair{3, 3},
                4,
                1
            }
        };

        model.SetRules(rules);

        Expect(
            model.RuleCount() == 2,
            "Model stores merge rules"
        );

        Expect(
            model.GetRules() == rules,
            "Stored rules preserved"
        );

        Expect(
            !model.Empty(),
            "Rules make model non-empty"
        );
    }

    /*
     * Symbol-level encoding.
     *
     * EncodeSymbols works directly on IDs and
     * therefore does not require byte fallback.
     */
    {
        BpeModel model;

        model.SetRules(
            {
                MergeRule{
                    Pair{1, 2},
                    3,
                    0
                },
                MergeRule{
                    Pair{3, 3},
                    4,
                    1
                }
            }
        );

        const std::vector<SymbolId>
            input{
                1, 2, 1, 2
            };

        const auto output =
            model.EncodeSymbols(
                input
            );

        Expect(
            output ==
                std::vector<SymbolId>{
                    4
                },
            "Model applies BPE rules"
        );
    }

    /*
     * Text inference on an unprepared model.
     *
     * BpeModel::Encode is now inference-only.
     * It must never grow vocabulary.
     *
     * Therefore a model without the universal
     * byte fallback alphabet is not ready for
     * text inference.
     */
    {
        BpeModel model;

        const usize before =
            model.VocabularySize();

        auto encoded =
            model.Encode("abc");

        Expect(
            encoded.Failed(),
            "Unprepared model text inference rejected"
        );

        Expect(
            model.VocabularySize() == before,
            "Failed inference does not grow vocabulary"
        );
    }

    /*
     * Frozen text encoding without merge rules.
     *
     * Known graphemes use learned IDs.
     */
    {
        BpeModel model;

        auto& vocabulary =
            model.GetVocabulary();

        const SymbolId a =
            vocabulary.Add("a");

        const SymbolId b =
            vocabulary.Add("b");

        const SymbolId c =
            vocabulary.Add("c");

        const bool fallback =
            vocabulary.EnsureByteFallback();

        const usize before =
            model.VocabularySize();

        auto encoded =
            model.Encode("abc");

        Expect(
            fallback &&
            encoded.Ok(),
            "Prepared text inference succeeds"
        );

        if (encoded.Ok())
        {
            Expect(
                encoded.Value() ==
                    std::vector<SymbolId>{
                        a,
                        b,
                        c
                    },
                "Known grapheme IDs preserved"
            );
        }

        Expect(
            model.VocabularySize() == before,
            "Known text inference keeps vocabulary frozen"
        );
    }

    /*
     * Text encoding with learned merge.
     */
    {
        BpeModel model;

        auto& vocabulary =
            model.GetVocabulary();

        const SymbolId a =
            vocabulary.Add("a");

        const SymbolId b =
            vocabulary.Add("b");

        const SymbolId ab =
            vocabulary.AddMerged(
                a,
                b
            );

        Expect(
            ab != InvalidSymbolId,
            "Merged symbol registered"
        );

        model.SetRules(
            {
                MergeRule{
                    Pair{a, b},
                    ab,
                    0
                }
            }
        );

        const bool fallback =
            vocabulary.EnsureByteFallback();

        const usize before =
            model.VocabularySize();

        auto encoded =
            model.Encode("abab");

        Expect(
            fallback &&
            encoded.Ok(),
            "Merged text inference succeeds"
        );

        if (encoded.Ok())
        {
            Expect(
                encoded.Value() ==
                    std::vector<SymbolId>{
                        ab,
                        ab
                    },
                "Learned merge applied to text"
            );
        }

        Expect(
            model.VocabularySize() == before,
            "Merged inference keeps vocabulary frozen"
        );
    }

    /*
     * Unicode learned grapheme encoding.
     */
    {
        BpeModel model;

        auto& vocabulary =
            model.GetVocabulary();

        const SymbolId ta =
            vocabulary.Add("த");

        const SymbolId mi =
            vocabulary.Add("மி");

        const SymbolId zh =
            vocabulary.Add("ழ்");

        const bool fallback =
            vocabulary.EnsureByteFallback();

        const usize before =
            model.VocabularySize();

        const std::string input =
            "தமிழ்";

        auto encoded =
            model.Encode(input);

        Expect(
            fallback &&
            encoded.Ok(),
            "Unicode text inference succeeds"
        );

        if (encoded.Ok())
        {
            Expect(
                encoded.Value() ==
                    std::vector<SymbolId>{
                        ta,
                        mi,
                        zh
                    },
                "Unicode learned grapheme IDs preserved"
            );
        }

        Expect(
            model.VocabularySize() == before,
            "Unicode inference keeps vocabulary frozen"
        );
    }

    /*
     * Unknown Unicode must use byte fallback.
     */
    {
        BpeModel model;

        auto& vocabulary =
            model.GetVocabulary();

        const bool fallback =
            vocabulary.EnsureByteFallback();

        const usize before =
            model.VocabularySize();

        const std::string input =
            "🧠";

        auto encoded =
            model.Encode(input);

        Expect(
            fallback &&
            encoded.Ok(),
            "Unknown Unicode inference succeeds"
        );

        if (encoded.Ok())
        {
            auto decoded =
                model.Decode(
                    encoded.Value()
                );

            Expect(
                decoded.Ok() &&
                decoded.Value() == input,
                "Unknown Unicode byte fallback round trip"
            );
        }

        Expect(
            model.VocabularySize() == before,
            "Unknown Unicode does not grow vocabulary"
        );
    }

    /*
     * Empty text remains valid even for a fresh
     * model because no symbol lookup is needed.
     */
    {
        BpeModel model;

        auto encoded =
            model.Encode("");

        Expect(
            encoded.Ok(),
            "Empty text encoding succeeds"
        );

        if (encoded.Ok())
        {
            Expect(
                encoded.Value().empty(),
                "Empty text produces empty symbols"
            );
        }
    }

    /*
     * Invalid UTF-8.
     *
     * Register fallback first so the failure is
     * specifically caused by UTF-8 validation,
     * not by an unprepared inference vocabulary.
     */
    {
        BpeModel model;

        const bool fallback =
            model.GetVocabulary().
                EnsureByteFallback();

        const std::string invalid(
            "\xC0\xAF",
            2
        );

        auto encoded =
            model.Encode(invalid);

        Expect(
            fallback &&
            encoded.Failed(),
            "Invalid UTF-8 rejected"
        );
    }

    const usize failed =
        tests_run -
        tests_passed;

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
