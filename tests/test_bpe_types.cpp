#include <iostream>
#include <unordered_map>

#include "bpe/pair.hpp"
#include "bpe/symbol.hpp"

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
        const Symbol symbol{42};

        Expect(
            symbol.id == 42,
            "Symbol stores ID"
        );

        Expect(
            symbol.Valid(),
            "Symbol valid"
        );
    }

    {
        const Symbol symbol{};

        Expect(
            !symbol.Valid(),
            "Default symbol invalid"
        );
    }

    {
        const Pair pair{10, 20};

        Expect(
            pair.left == 10 &&
            pair.right == 20,
            "Pair stores symbols"
        );

        Expect(
            pair.Valid(),
            "Pair valid"
        );
    }

    {
        const Pair a{10, 20};
        const Pair b{10, 20};
        const Pair c{20, 10};

        Expect(
            a == b,
            "Equal pairs compare equal"
        );

        Expect(
            !(a == c),
            "Pair order preserved"
        );
    }

    {
        std::unordered_map<
            Pair,
            usize,
            PairHash
        > frequencies;

        frequencies[Pair{1, 2}] += 1;
        frequencies[Pair{1, 2}] += 1;
        frequencies[Pair{2, 3}] += 1;

        Expect(
            frequencies[Pair{1, 2}] == 2,
            "Pair hash frequency"
        );

        Expect(
            frequencies[Pair{2, 3}] == 1,
            "Distinct pair frequency"
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
