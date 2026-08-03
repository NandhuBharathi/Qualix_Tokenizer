#include <iostream>
#include <vector>

#include "bpe/trainer.hpp"

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
        const std::vector<SymbolId>
            symbols{
                1, 2, 1, 2, 3
            };

        const TrainerConfig config{
            .max_merges = 10,
            .min_frequency = 2
        };

        const auto result =
            BpeTrainer::Train(
                symbols,
                4,
                config
            );

        Expect(
            result.rules.size() == 1,
            "One merge learned"
        );

        Expect(
            result.rules.size() == 1 &&
            result.rules[0].pair ==
                Pair{1, 2},
            "Most frequent pair learned"
        );

        Expect(
            result.rules.size() == 1 &&
            result.rules[0].merged_symbol ==
                4,
            "First new ID assigned"
        );

        Expect(
            result.symbols ==
                std::vector<SymbolId>{
                    4, 4, 3
                },
            "Training sequence merged"
        );
    }

    {
        /*
         * Two rounds:
         *
         * [1,2,1,2,1,2]
         *
         * (1,2) -> 3
         *
         * [3,3,3]
         *
         * (3,3) occurs twice -> 4
         *
         * Non-overlapping merge:
         *
         * [4,3]
         */
        const std::vector<SymbolId>
            symbols{
                1, 2, 1, 2, 1, 2
            };

        const TrainerConfig config{
            .max_merges = 2,
            .min_frequency = 2
        };

        const auto result =
            BpeTrainer::Train(
                symbols,
                3,
                config
            );

        Expect(
            result.rules.size() == 2,
            "Two merges learned"
        );

        Expect(
            result.rules.size() == 2 &&
            result.rules[0].pair ==
                Pair{1, 2} &&
            result.rules[0].merged_symbol == 3,
            "First merge rule correct"
        );

        Expect(
            result.rules.size() == 2 &&
            result.rules[1].pair ==
                Pair{3, 3} &&
            result.rules[1].merged_symbol == 4,
            "Second merge rule correct"
        );

        Expect(
            result.symbols ==
                std::vector<SymbolId>{
                    4, 3
                },
            "Repeated training rounds correct"
        );
    }

    {
        const std::vector<SymbolId>
            symbols{
                10, 20, 10, 20
            };

        const TrainerConfig config{
            .max_merges = 1,
            .min_frequency = 2
        };

        const auto result =
            BpeTrainer::Train(
                symbols,
                100,
                config
            );

        Expect(
            result.rules.size() == 1 &&
            result.rules[0].merged_symbol == 100,
            "Existing IDs preserved"
        );

        Expect(
            result.symbols ==
                std::vector<SymbolId>{
                    100, 100
                },
            "New ID appended logically"
        );
    }

    {
        const std::vector<SymbolId>
            symbols{
                1, 2, 3, 4
            };

        const TrainerConfig config{
            .max_merges = 10,
            .min_frequency = 2
        };

        const auto result =
            BpeTrainer::Train(
                symbols,
                5,
                config
            );

        Expect(
            result.rules.empty(),
            "Rare pairs not merged"
        );

        Expect(
            result.symbols == symbols,
            "Sequence unchanged without merge"
        );
    }

    {
        const std::vector<SymbolId>
            symbols{
                1, 2, 1, 2
            };

        const TrainerConfig config{
            .max_merges = 0,
            .min_frequency = 2
        };

        const auto result =
            BpeTrainer::Train(
                symbols,
                3,
                config
            );

        Expect(
            result.rules.empty(),
            "Zero merge limit respected"
        );

        Expect(
            result.symbols == symbols,
            "Zero merge leaves sequence unchanged"
        );
    }

    {
        const std::vector<SymbolId>
            symbols;

        const TrainerConfig config{
            .max_merges = 10,
            .min_frequency = 2
        };

        const auto result =
            BpeTrainer::Train(
                symbols,
                1,
                config
            );

        Expect(
            result.rules.empty(),
            "Empty input safe"
        );

        Expect(
            result.symbols.empty(),
            "Empty result safe"
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
