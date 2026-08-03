#include <iostream>
#include <vector>

#include "bpe/encoder.hpp"

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
     * Basic single merge.
     */
    {
        const std::vector<SymbolId>
            symbols{
                1, 2, 1, 2, 3
            };

        const std::vector<MergeRule>
            rules{
                MergeRule{
                    Pair{1, 2},
                    4,
                    0
                }
            };

        const auto encoded =
            BpeEncoder::Encode(
                symbols,
                rules
            );

        Expect(
            encoded ==
                std::vector<SymbolId>{
                    4, 4, 3
                },
            "Single learned rule applied"
        );
    }

    /*
     * Multiple training-order merges.
     *
     * [1,2,1,2,1,2]
     *
     * rank 0:
     * (1,2) -> 3
     *
     * [3,3,3]
     *
     * rank 1:
     * (3,3) -> 4
     *
     * [4,3]
     */
    {
        const std::vector<SymbolId>
            symbols{
                1, 2, 1, 2, 1, 2
            };

        const std::vector<MergeRule>
            rules{
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

        const auto encoded =
            BpeEncoder::Encode(
                symbols,
                rules
            );

        Expect(
            encoded ==
                std::vector<SymbolId>{
                    4, 3
                },
            "Multiple merge rules applied"
        );
    }

    /*
     * Rules intentionally supplied in
     * reverse order.
     *
     * Encoder must respect rank.
     */
    {
        const std::vector<SymbolId>
            symbols{
                1, 2, 1, 2
            };

        const std::vector<MergeRule>
            rules{
                MergeRule{
                    Pair{3, 3},
                    4,
                    1
                },
                MergeRule{
                    Pair{1, 2},
                    3,
                    0
                }
            };

        const auto encoded =
            BpeEncoder::Encode(
                symbols,
                rules
            );

        Expect(
            encoded ==
                std::vector<SymbolId>{
                    4
                },
            "Merge rank overrides storage order"
        );
    }

    /*
     * No applicable pair.
     */
    {
        const std::vector<SymbolId>
            symbols{
                10, 20, 30
            };

        const std::vector<MergeRule>
            rules{
                MergeRule{
                    Pair{1, 2},
                    100,
                    0
                }
            };

        const auto encoded =
            BpeEncoder::Encode(
                symbols,
                rules
            );

        Expect(
            encoded == symbols,
            "Unknown pair leaves sequence unchanged"
        );
    }

    /*
     * Empty rules.
     */
    {
        const std::vector<SymbolId>
            symbols{
                1, 2, 3
            };

        const std::vector<MergeRule>
            rules;

        const auto encoded =
            BpeEncoder::Encode(
                symbols,
                rules
            );

        Expect(
            encoded == symbols,
            "Empty rule set safe"
        );
    }

    /*
     * Empty input.
     */
    {
        const std::vector<SymbolId>
            symbols;

        const std::vector<MergeRule>
            rules{
                MergeRule{
                    Pair{1, 2},
                    3,
                    0
                }
            };

        const auto encoded =
            BpeEncoder::Encode(
                symbols,
                rules
            );

        Expect(
            encoded.empty(),
            "Empty input safe"
        );
    }

    /*
     * Single symbol.
     */
    {
        const std::vector<SymbolId>
            symbols{
                7
            };

        const std::vector<MergeRule>
            rules{
                MergeRule{
                    Pair{7, 7},
                    8,
                    0
                }
            };

        const auto encoded =
            BpeEncoder::Encode(
                symbols,
                rules
            );

        Expect(
            encoded ==
                std::vector<SymbolId>{
                    7
                },
            "Single symbol safe"
        );
    }

    /*
     * Invalid rules must not affect
     * encoding.
     */
    {
        const std::vector<SymbolId>
            symbols{
                1, 2
            };

        const std::vector<MergeRule>
            rules{
                MergeRule{
                    Pair{},
                    InvalidSymbolId,
                    0
                },
                MergeRule{
                    Pair{1, 2},
                    3,
                    1
                }
            };

        const auto encoded =
            BpeEncoder::Encode(
                symbols,
                rules
            );

        Expect(
            encoded ==
                std::vector<SymbolId>{
                    3
                },
            "Invalid rules ignored"
        );
    }

    /*
     * Non-overlapping behavior inherited
     * from Merger.
     *
     * [1,1,1]
     * (1,1) -> 2
     *
     * Result must be [2,1], not [2].
     */
    {
        const std::vector<SymbolId>
            symbols{
                1, 1, 1
            };

        const std::vector<MergeRule>
            rules{
                MergeRule{
                    Pair{1, 1},
                    2,
                    0
                }
            };

        const auto encoded =
            BpeEncoder::Encode(
                symbols,
                rules
            );

        Expect(
            encoded ==
                std::vector<SymbolId>{
                    2, 1
                },
            "Overlapping pair merged safely"
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
