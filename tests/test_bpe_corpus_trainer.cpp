#include <iostream>
#include <vector>

#include "bpe/corpus_trainer.hpp"

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

        const SymbolId a =
            vocabulary.Add("a");

        const SymbolId b =
            vocabulary.Add("b");

        const SymbolId c =
            vocabulary.Add("c");

        const SymbolCorpus corpus{
            {a, b, a, b},
            {a, b, c},
            {c, a, b}
        };

        const TrainerConfig config{
            .max_merges = 1,
            .min_frequency = 2
        };

        const auto result =
            BpeCorpusTrainer::Train(
                corpus,
                vocabulary,
                config
            );

        Expect(
            result.rules.size() == 1,
            "One global merge learned"
        );

        Expect(
            result.rules.size() == 1 &&
            result.rules[0].pair ==
                Pair{a, b},
            "Global most frequent pair selected"
        );

        Expect(
            result.rules.size() == 1 &&
            result.rules[0].rank == 0,
            "First merge rank zero"
        );

        const SymbolId ab =
            result.rules.empty()
                ? InvalidSymbolId
                : result.rules[0].merged_symbol;

        const auto text =
            vocabulary.Find(ab);

        Expect(
            text.has_value() &&
            *text == "ab",
            "Merged text registered in vocabulary"
        );

        Expect(
            result.corpus.size() == 3,
            "Corpus sequence count preserved"
        );

        Expect(
            result.corpus.size() == 3 &&
            result.corpus[0] ==
                SymbolSequence{
                    ab, ab
                },
            "Merge applied to first sequence"
        );

        Expect(
            result.corpus.size() == 3 &&
            result.corpus[1] ==
                SymbolSequence{
                    ab, c
                },
            "Merge applied to second sequence"
        );

        Expect(
            result.corpus.size() == 3 &&
            result.corpus[2] ==
                SymbolSequence{
                    c, ab
                },
            "Merge applied to third sequence"
        );
    }

    {
        /*
         * Boundary safety test.
         *
         * If sequences were incorrectly flattened:
         *
         * [a] [b]
         *
         * would create pair (a,b).
         *
         * Correct corpus training must NOT see
         * that pair.
         */
        Vocabulary vocabulary;

        const SymbolId a =
            vocabulary.Add("a");

        const SymbolId b =
            vocabulary.Add("b");

        const SymbolCorpus corpus{
            {a},
            {b},
            {a},
            {b}
        };

        const TrainerConfig config{
            .max_merges = 10,
            .min_frequency = 2
        };

        const auto result =
            BpeCorpusTrainer::Train(
                corpus,
                vocabulary,
                config
            );

        Expect(
            result.rules.empty(),
            "Sequence boundaries block cross-span pairs"
        );

        Expect(
            result.corpus == corpus,
            "Boundary-only corpus unchanged"
        );

        Expect(
            vocabulary.Size() == 2,
            "Boundary test does not grow vocabulary"
        );
    }

    {
        /*
         * Two rounds:
         *
         * [a,b,a,b,a,b]
         *
         * round 0:
         *   (a,b) -> ab
         *
         * [ab,ab,ab]
         *
         * round 1:
         *   (ab,ab) -> abab
         *
         * Non-overlapping merge gives:
         *
         * [abab,ab]
         */
        Vocabulary vocabulary;

        const SymbolId a =
            vocabulary.Add("a");

        const SymbolId b =
            vocabulary.Add("b");

        const SymbolCorpus corpus{
            {
                a, b,
                a, b,
                a, b
            }
        };

        const TrainerConfig config{
            .max_merges = 2,
            .min_frequency = 2
        };

        const auto result =
            BpeCorpusTrainer::Train(
                corpus,
                vocabulary,
                config
            );

        Expect(
            result.rules.size() == 2,
            "Multiple corpus rounds learned"
        );

        Expect(
            result.rules.size() == 2 &&
            result.rules[0].rank == 0 &&
            result.rules[1].rank == 1,
            "Merge ranks deterministic"
        );

        if (result.rules.size() == 2)
        {
            const SymbolId ab =
                result.rules[0].merged_symbol;

            const SymbolId abab =
                result.rules[1].merged_symbol;

            const auto ab_text =
                vocabulary.Find(ab);

            const auto abab_text =
                vocabulary.Find(abab);

            Expect(
                ab_text.has_value() &&
                *ab_text == "ab",
                "First merged vocabulary text correct"
            );

            Expect(
                abab_text.has_value() &&
                *abab_text == "abab",
                "Recursive merged vocabulary text correct"
            );

            Expect(
                result.corpus.size() == 1 &&
                result.corpus[0] ==
                    SymbolSequence{
                        abab,
                        ab
                    },
                "Multiple corpus merges applied"
            );
        }
        else
        {
            Expect(
                false,
                "First merged vocabulary text correct"
            );

            Expect(
                false,
                "Recursive merged vocabulary text correct"
            );

            Expect(
                false,
                "Multiple corpus merges applied"
            );
        }
    }

    {
        /*
         * Global frequency must be accumulated
         * across sequences.
         *
         * Each individual sequence contains
         * (a,b) only once, but globally it occurs
         * three times.
         */
        Vocabulary vocabulary;

        const SymbolId a =
            vocabulary.Add("a");

        const SymbolId b =
            vocabulary.Add("b");

        const SymbolId x =
            vocabulary.Add("x");

        const SymbolId y =
            vocabulary.Add("y");

        const SymbolCorpus corpus{
            {a, b, x},
            {y, a, b},
            {a, b}
        };

        const TrainerConfig config{
            .max_merges = 1,
            .min_frequency = 3
        };

        const auto result =
            BpeCorpusTrainer::Train(
                corpus,
                vocabulary,
                config
            );

        Expect(
            result.rules.size() == 1 &&
            result.rules[0].pair ==
                Pair{a, b},
            "Pair frequencies accumulated globally"
        );
    }

    {
        Vocabulary vocabulary;

        const SymbolId a =
            vocabulary.Add("a");

        const SymbolId b =
            vocabulary.Add("b");

        const SymbolCorpus corpus{
            {a, b, a, b}
        };

        const TrainerConfig config{
            .max_merges = 0,
            .min_frequency = 2
        };

        const auto result =
            BpeCorpusTrainer::Train(
                corpus,
                vocabulary,
                config
            );

        Expect(
            result.rules.empty(),
            "Zero merge limit respected"
        );

        Expect(
            result.corpus == corpus,
            "Zero merge corpus unchanged"
        );

        Expect(
            vocabulary.Size() == 2,
            "Zero merge does not grow vocabulary"
        );
    }

    {
        Vocabulary vocabulary;

        const SymbolCorpus corpus;

        const TrainerConfig config{
            .max_merges = 10,
            .min_frequency = 2
        };

        const auto result =
            BpeCorpusTrainer::Train(
                corpus,
                vocabulary,
                config
            );

        Expect(
            result.rules.empty(),
            "Empty corpus has no rules"
        );

        Expect(
            result.corpus.empty(),
            "Empty corpus safe"
        );
    }

    {
        /*
         * Deterministic tie-break.
         *
         * Frequencies:
         *
         * (a,c) = 1
         * (c,b) = 1
         * (b,c) = 1
         *
         * Symbol IDs:
         *
         * a=1
         * b=2
         * c=3
         *
         * Lexicographically smallest pair:
         *
         * (1,3) == (a,c)
         */
        Vocabulary vocabulary;

        const SymbolId a =
            vocabulary.Add("a");

        const SymbolId b =
            vocabulary.Add("b");

        const SymbolId c =
            vocabulary.Add("c");

        const SymbolCorpus corpus{
            {a, c},
            {c, b},
            {b, c}
        };

        const TrainerConfig config{
            .max_merges = 1,
            .min_frequency = 1
        };

        const auto result =
            BpeCorpusTrainer::Train(
                corpus,
                vocabulary,
                config
            );

        Expect(
            result.rules.size() == 1 &&
            result.rules[0].pair ==
                Pair{a, c},
            "Corpus tie break deterministic"
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
