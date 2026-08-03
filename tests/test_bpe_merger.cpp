#include <iostream>
#include <vector>

#include "bpe/merger.hpp"

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
            symbols;

        const auto result =
            Merger::Merge(
                symbols,
                Pair{1, 2},
                10
            );

        Expect(
            result.empty(),
            "Empty sequence remains empty"
        );
    }

    {
        const std::vector<SymbolId>
            symbols{
                1, 2, 1, 2, 3
            };

        const auto result =
            Merger::Merge(
                symbols,
                Pair{1, 2},
                10
            );

        const std::vector<SymbolId>
            expected{
                10, 10, 3
            };

        Expect(
            result == expected,
            "Repeated pair merged"
        );
    }

    {
        /*
         * Counting may observe overlapping pairs,
         * but a merge operation itself is
         * left-to-right and non-overlapping.
         *
         * [7,7,7,7]
         *
         * (0,1) -> 8
         * (2,3) -> 8
         */
        const std::vector<SymbolId>
            symbols{
                7, 7, 7, 7
            };

        const auto result =
            Merger::Merge(
                symbols,
                Pair{7, 7},
                8
            );

        const std::vector<SymbolId>
            expected{
                8, 8
            };

        Expect(
            result == expected,
            "Self pair merges non-overlapping"
        );
    }

    {
        const std::vector<SymbolId>
            symbols{
                7, 7, 7
            };

        const auto result =
            Merger::Merge(
                symbols,
                Pair{7, 7},
                8
            );

        const std::vector<SymbolId>
            expected{
                8, 7
            };

        Expect(
            result == expected,
            "Odd self pair preserves tail"
        );
    }

    {
        const std::vector<SymbolId>
            symbols{
                1, 2, 3, 4
            };

        const auto result =
            Merger::Merge(
                symbols,
                Pair{9, 9},
                10
            );

        Expect(
            result == symbols,
            "Missing pair leaves sequence unchanged"
        );
    }

    {
        const std::vector<SymbolId>
            symbols{
                1, 2, 3
            };

        const auto result =
            Merger::Merge(
                symbols,
                Pair{},
                10
            );

        Expect(
            result == symbols,
            "Invalid pair leaves sequence unchanged"
        );
    }

    {
        const std::vector<SymbolId>
            symbols{
                1, 2, 3
            };

        const auto result =
            Merger::Merge(
                symbols,
                Pair{1, 2},
                InvalidSymbolId
            );

        Expect(
            result == symbols,
            "Invalid merged symbol rejected"
        );
    }

    {
        /*
         * Direction must be preserved.
         */
        const std::vector<SymbolId>
            symbols{
                1, 2, 2, 1
            };

        const auto result =
            Merger::Merge(
                symbols,
                Pair{1, 2},
                9
            );

        const std::vector<SymbolId>
            expected{
                9, 2, 1
            };

        Expect(
            result == expected,
            "Pair direction preserved"
        );
    }

    {
        /*
         * Newly-created symbols are not recursively
         * re-merged during the same merge pass.
         */
        const std::vector<SymbolId>
            symbols{
                1, 2, 2
            };

        const auto result =
            Merger::Merge(
                symbols,
                Pair{1, 2},
                1
            );

        const std::vector<SymbolId>
            expected{
                1, 2
            };

        Expect(
            result == expected,
            "Single pass does not recursively merge output"
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
