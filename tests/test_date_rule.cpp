#include <iostream>
#include <string>
#include <string_view>

#include "rules/date_rule.hpp"
#include "rules/rule_type.hpp"

using namespace qualix;
using namespace qualix::rules;

namespace {

int tests_run = 0;
int tests_passed = 0;
int tests_failed = 0;

void Expect(
    bool condition,
    std::string_view name
)
{
    ++tests_run;

    if (condition)
    {
        ++tests_passed;
        std::cout
            << "[PASS] "
            << name
            << "\n";
    }
    else
    {
        ++tests_failed;
        std::cout
            << "[FAIL] "
            << name
            << "\n";
    }
}

void ExpectDate(
    std::string_view input,
    std::string_view expected,
    std::string_view name
)
{
    DateRule rule;

    const auto match =
        rule.Match(input, 0);

    Expect(
        match.Matched(),
        name
    );

    if (!match.Matched())
        return;

    Expect(
        match.type ==
            RuleType::Date,
        "Date match type"
    );

    Expect(
        input.substr(
            match.byte_start,
            match.byte_length
        ) == expected,
        "Date match text"
    );
}

void ExpectRejected(
    std::string_view input,
    std::string_view name
)
{
    DateRule rule;

    const auto match =
        rule.Match(input, 0);

    Expect(
        !match.Matched(),
        name
    );
}

} // namespace

int main()
{
    // D/M/Y and DD/MM/YYYY
    ExpectDate(
        "12/3/25",
        "12/3/25",
        "Short slash date"
    );

    ExpectDate(
        "12/03/2025",
        "12/03/2025",
        "Full slash date"
    );

    ExpectDate(
        "1/1/2026",
        "1/1/2026",
        "Single digit day month"
    );

    // ISO
    ExpectDate(
        "2025-03-12",
        "2025-03-12",
        "ISO date"
    );

    ExpectDate(
        "2026-08-01",
        "2026-08-01",
        "ISO current style"
    );

    // Dot-separated
    ExpectDate(
        "12.03.2025",
        "12.03.2025",
        "Dot date"
    );

    // Valid boundaries
    ExpectDate(
        "31/12/2025",
        "31/12/2025",
        "End of year"
    );

    ExpectDate(
        "29/02/2024",
        "29/02/2024",
        "Leap year date"
    );

    // Punctuation must not become part of date
    ExpectDate(
        "12/03/2025.",
        "12/03/2025",
        "Trailing punctuation excluded"
    );

    ExpectDate(
        "2025-03-12,",
        "2025-03-12",
        "ISO trailing comma excluded"
    );

    // Invalid dates
    ExpectRejected(
        "32/01/2025",
        "Invalid day rejected"
    );

    ExpectRejected(
        "00/01/2025",
        "Zero day rejected"
    );

    ExpectRejected(
        "12/00/2025",
        "Zero month rejected"
    );

    ExpectRejected(
        "12/13/2025",
        "Invalid month rejected"
    );

    ExpectRejected(
        "31/02/2025",
        "Invalid February date rejected"
    );

    ExpectRejected(
        "29/02/2025",
        "Non leap February rejected"
    );

    ExpectRejected(
        "2025-02-29",
        "Invalid ISO leap date rejected"
    );

    // Incomplete
    ExpectRejected(
        "12/03",
        "Incomplete date rejected"
    );

    ExpectRejected(
        "2025-03",
        "Incomplete ISO date rejected"
    );

    // Identifier boundaries
    ExpectRejected(
        "abc12/03/2025",
        "Date inside identifier rejected"
    );

    ExpectRejected(
        "12/03/2025abc",
        "Date identifier continuation rejected"
    );

    // Random separators
    ExpectRejected(
        "12//03/2025",
        "Double separator rejected"
    );

    ExpectRejected(
        "12-03/2025",
        "Mixed separator rejected"
    );

    DateRule rule;

    Expect(
        rule.Type() ==
            RuleType::Date,
        "Date rule reports type"
    );

    std::cout
        << "\n================================\n"
        << "Tests Run    : "
        << tests_run
        << "\n"
        << "Tests Passed : "
        << tests_passed
        << "\n"
        << "Tests Failed : "
        << tests_failed
        << "\n"
        << "================================\n";

    return tests_failed == 0
        ? 0
        : 1;
}
