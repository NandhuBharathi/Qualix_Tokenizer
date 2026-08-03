#include <iostream>
#include <limits>

#include "bpe/symbol_allocator.hpp"

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
        SymbolAllocator allocator{6};

        Expect(
            allocator.Next() == 6,
            "Allocator stores first available ID"
        );

        const auto a =
            allocator.Allocate();

        const auto b =
            allocator.Allocate();

        const auto c =
            allocator.Allocate();

        Expect(
            a.has_value() && *a == 6,
            "First allocation correct"
        );

        Expect(
            b.has_value() && *b == 7,
            "Second allocation monotonic"
        );

        Expect(
            c.has_value() && *c == 8,
            "Third allocation monotonic"
        );

        Expect(
            allocator.Next() == 9,
            "Next ID advances"
        );
    }

    {
        SymbolAllocator allocator{
            InvalidSymbolId
        };

        const auto result =
            allocator.Allocate();

        Expect(
            !result.has_value(),
            "Invalid start rejected"
        );
    }

    {
        constexpr SymbolId max_id =
            std::numeric_limits<
                SymbolId
            >::max();

        SymbolAllocator allocator{
            max_id
        };

        const auto last =
            allocator.Allocate();

        Expect(
            last.has_value() &&
            *last == max_id,
            "Maximum ID can be allocated"
        );

        Expect(
            allocator.Next() ==
                InvalidSymbolId,
            "Allocator marks exhaustion"
        );

        const auto exhausted =
            allocator.Allocate();

        Expect(
            !exhausted.has_value(),
            "Exhausted allocator rejects allocation"
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
