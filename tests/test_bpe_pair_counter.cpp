#include <iostream>
#include <vector>

#include "bpe/pair_counter.hpp"

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

usize Frequency(
    const PairFrequencies& frequencies,
    Pair pair
)
{
    const auto it =
        frequencies.find(pair);

    if (it == frequencies.end())
        return 0;

    return it->second;
}

} // namespace

int main()
{
    {
        const std::vector<SymbolId>
            symbols;

        const auto frequencies =
            PairCounter::Count(symbols);

        Expect(
            frequencies.empty(),
            "Empty sequence has no pairs"
        );

        Expect(
            !PairCounter::MostFrequent(
                frequencies
            ).has_value(),
            "Empty frequencies have no best pair"
        );
    }

    {
        const std::vector<SymbolId>
            symbols{42};

        const auto frequencies =
            PairCounter::Count(symbols);

        Expect(
            frequencies.empty(),
            "Single symbol has no pairs"
        );
    }

    {
        const std::vector<SymbolId>
            symbols{
                1, 2, 1, 2, 3
            };

        const auto frequencies =
            PairCounter::Count(symbols);

        Expect(
            frequencies.size() == 3,
            "Distinct pair count"
        );

        Expect(
            Frequency(
                frequencies,
                Pair{1, 2}
            ) == 2,
            "Repeated pair frequency"
        );

        Expect(
            Frequency(
                frequencies,
                Pair{2, 1}
            ) == 1,
            "Reverse pair frequency"
        );

        Expect(
            Frequency(
                frequencies,
                Pair{2, 3}
            ) == 1,
            "Final pair frequency"
        );

        const auto best =
            PairCounter::MostFrequent(
                frequencies
            );

        Expect(
            best.has_value(),
            "Best pair exists"
        );

        Expect(
            best.has_value() &&
            best->pair == Pair{1, 2},
            "Most frequent pair selected"
        );

        Expect(
            best.has_value() &&
            best->frequency == 2,
            "Most frequent count correct"
        );
    }

    {
        /*
         * Adjacent pairs overlap during counting:
         *
         * [7, 7, 7, 7]
         *
         * positions:
         *   (0,1)
         *   (1,2)
         *   (2,3)
         *
         * Therefore (7,7) occurs three times.
         */
        const std::vector<SymbolId>
            symbols{
                7, 7, 7, 7
            };

        const auto frequencies =
            PairCounter::Count(symbols);

        Expect(
            frequencies.size() == 1,
            "Overlapping pair one distinct pair"
        );

        Expect(
            Frequency(
                frequencies,
                Pair{7, 7}
            ) == 3,
            "Overlapping pair counted"
        );
    }

    {
        /*
         * Tie:
         *
         * (5,9) = 1
         * (9,3) = 1
         * (3,8) = 1
         *
         * Deterministic tie-break chooses the
         * lexicographically smallest SymbolId pair:
         *
         * (3,8)
         */
        const std::vector<SymbolId>
            symbols{
                5, 9, 3, 8
            };

        const auto frequencies =
            PairCounter::Count(symbols);

        const auto best =
            PairCounter::MostFrequent(
                frequencies
            );

        Expect(
            best.has_value() &&
            best->pair == Pair{3, 8},
            "Tie break deterministic"
        );

        Expect(
            best.has_value() &&
            best->frequency == 1,
            "Tie frequency correct"
        );
    }

    {
        /*
         * Pair direction must remain significant.
         */
        const std::vector<SymbolId>
            symbols{
                1, 2, 2, 1
            };

        const auto frequencies =
            PairCounter::Count(symbols);

        Expect(
            Frequency(
                frequencies,
                Pair{1, 2}
            ) == 1,
            "Forward pair preserved"
        );

        Expect(
            Frequency(
                frequencies,
                Pair{2, 1}
            ) == 1,
            "Reverse pair preserved"
        );

        Expect(
            Frequency(
                frequencies,
                Pair{2, 2}
            ) == 1,
            "Self pair preserved"
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
