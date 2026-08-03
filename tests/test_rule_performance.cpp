
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "rules/rule.hpp"
#include "rules/url_rule.hpp"
#include "rules/email_rule.hpp"
#include "rules/currency_rule.hpp"
#include "rules/date_rule.hpp"
#include "rules/time_rule.hpp"
#include "rules/phone_rule.hpp"
#include "rules/measurement_rule.hpp"
#include "rules/math_rule.hpp"
#include "rules/percentage_rule.hpp"
#include "rules/number_rule.hpp"
#include "unicode/grapheme_segmenter.hpp"

using namespace qualix;
using namespace qualix::rules;

struct Entry
{
    const char* name;
    std::unique_ptr<Rule> rule;
    double seconds = 0.0;
    usize calls = 0;
    usize matches = 0;
};

int main()
{
    std::string unit =
        "Hello world 12345 test@example.com "
        "https://example.com/path 12.5% $99.50 "
        "2026-08-02 10:30 +91-9876543210 "
        "25kg x+y=10 தமிழ் வணக்கம் ";

    std::string corpus;
    corpus.reserve(unit.size() * 1500);

    for (int i = 0; i < 1500; ++i)
        corpus += unit;

    auto segmented =
        unicode::GraphemeSegmenter::Segment(corpus);

    if (segmented.Failed())
    {
        std::cout << "[FAIL] SEGMENT\n";
        return 1;
    }

    const auto& graphemes =
        segmented.Value();

    std::vector<Entry> rules;

    rules.push_back({"Email", std::make_unique<EmailRule>()});
    rules.push_back({"Percentage", std::make_unique<PercentageRule>()});
    rules.push_back({"URL", std::make_unique<UrlRule>()});
    rules.push_back({"Date", std::make_unique<DateRule>()});
    rules.push_back({"Time", std::make_unique<TimeRule>()});
    rules.push_back({"Currency", std::make_unique<CurrencyRule>()});
    rules.push_back({"Measurement", std::make_unique<MeasurementRule>()});
    rules.push_back({"Phone", std::make_unique<PhoneRule>()});
    rules.push_back({"Math", std::make_unique<MathRule>()});
    rules.push_back({"Number", std::make_unique<NumberRule>()});

    using Clock = std::chrono::steady_clock;

    for (auto& entry : rules)
    {
        const auto start = Clock::now();

        for (const auto& grapheme : graphemes)
        {
            const auto match =
                entry.rule->Match(
                    corpus,
                    grapheme.byte_start
                );

            ++entry.calls;

            if (match.Matched())
                ++entry.matches;
        }

        const auto end = Clock::now();

        entry.seconds =
            std::chrono::duration<double>(
                end - start
            ).count();
    }

    double total = 0.0;

    for (const auto& entry : rules)
        total += entry.seconds;

    std::cout
        << "\n============================================================\n"
        << "QUALIX — #85 PER-RULE PERFORMANCE\n"
        << "============================================================\n"
        << "Corpus     : "
        << std::fixed << std::setprecision(3)
        << corpus.size() / 1000000.0
        << " MB\n"
        << "Graphemes  : "
        << graphemes.size()
        << "\n\n";

    std::cout
        << std::left
        << std::setw(14) << "Rule"
        << std::right
        << std::setw(12) << "Calls"
        << std::setw(12) << "Matches"
        << std::setw(14) << "Time"
        << '\n';

    for (const auto& entry : rules)
    {
        std::cout
            << std::left
            << std::setw(14)
            << entry.name
            << std::right
            << std::setw(12)
            << entry.calls
            << std::setw(12)
            << entry.matches
            << std::setw(11)
            << std::fixed
            << std::setprecision(3)
            << entry.seconds
            << " s\n";
    }

    std::cout
        << "------------------------------------------------------------\n"
        << "Total rule time : "
        << std::fixed
        << std::setprecision(3)
        << total
        << " s\n"
        << "============================================================\n";

    return 0;
}
