#include <iostream>

#include "bpe/merge_rule.hpp"

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
        const MergeRule rule{
            Pair{10, 20},
            100,
            5
        };

        Expect(
            rule.pair == Pair{10, 20},
            "Merge rule stores pair"
        );

        Expect(
            rule.merged_symbol == 100,
            "Merge rule stores output symbol"
        );

        Expect(
            rule.rank == 5,
            "Merge rule stores rank"
        );

        Expect(
            rule.Valid(),
            "Merge rule valid"
        );
    }

    {
        const MergeRule rule{};

        Expect(
            !rule.Valid(),
            "Default merge rule invalid"
        );
    }

    {
        const MergeRule rule{
            Pair{},
            100,
            0
        };

        Expect(
            !rule.Valid(),
            "Invalid pair rejected"
        );
    }

    {
        const MergeRule rule{
            Pair{1, 2},
            InvalidSymbolId,
            0
        };

        Expect(
            !rule.Valid(),
            "Invalid output symbol rejected"
        );
    }

    {
        const MergeRule a{
            Pair{1, 2},
            10,
            0
        };

        const MergeRule b{
            Pair{1, 2},
            10,
            0
        };

        Expect(
            a == b,
            "Equal merge rules compare equal"
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
