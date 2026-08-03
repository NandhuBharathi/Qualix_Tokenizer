
#include <iomanip>
#include <iostream>
#include <string>

#include "pretokenizer/pretokenizer.hpp"
#include "rules/rule.hpp"

using namespace qualix;

namespace qualix::rules::router_profile
{
void Reset() noexcept;

unsigned long long GetEligible(
    RuleType
) noexcept;

unsigned long long GetSkipped(
    RuleType
) noexcept;

unsigned long long GetCalled(
    RuleType
) noexcept;

unsigned long long GetMatched(
    RuleType
) noexcept;
}

namespace
{

struct Info
{
    const char* name;
    rules::RuleType type;
};

constexpr Info rules_list[] =
{
    {"Email",       rules::RuleType::Email},
    {"Percentage",  rules::RuleType::Percentage},
    {"URL",         rules::RuleType::Url},
    {"Date",        rules::RuleType::Date},
    {"Time",        rules::RuleType::Time},
    {"Currency",    rules::RuleType::Currency},
    {"Measurement", rules::RuleType::Measurement},
    {"Phone",       rules::RuleType::Phone},
    {"Math",        rules::RuleType::Math},
    {"Number",      rules::RuleType::Number}
};

std::string MakeCorpus(
    std::size_t bytes
)
{
    const std::string block =
        "Hello Qualix tokenizer ordinary text words. "
        "Price ₹1,25,000 USD 500 EUR 1,000.50. "
        "75% 12.5kg 180cm 42 3.14159. "
        "2026-08-02 11:45 PM "
        "test@example.com "
        "https://example.com/path?q=1 "
        "+91 9876543210 x+2=10 "
        "தமிழ் மொழி tokenizer test.\n";

    std::string corpus;
    corpus.reserve(bytes + block.size());

    while (corpus.size() < bytes)
        corpus += block;

    corpus.resize(bytes);

    return corpus;
}

}

int main()
{
    constexpr std::size_t bytes =
        10ULL * 1024ULL * 1024ULL;

    const std::string corpus =
        MakeCorpus(bytes);

    rules::router_profile::Reset();

    const auto result =
        pretokenizer::PreTokenizer::Split(
            corpus
        );

    if (result.Failed())
    {
        std::cout
            << "[FAIL] PRETOKENIZER\n";
        return 1;
    }

    unsigned long long eligible_total = 0;
    unsigned long long skipped_total = 0;
    unsigned long long called_total = 0;
    unsigned long long matched_total = 0;

    std::cout
        << "============================================================\n"
        << "QUALIX — #96 ROUTER DISPATCH COUNTERS\n"
        << "============================================================\n"
        << "[PASS] BUILD\n"
        << "Corpus : 10.000 MB\n\n";

    std::cout
        << std::left
        << std::setw(14) << "Rule"
        << std::right
        << std::setw(13) << "Eligible"
        << std::setw(13) << "Skipped"
        << std::setw(13) << "Called"
        << std::setw(13) << "Matched"
        << '\n';

    std::cout
        << std::string(66, '-')
        << '\n';

    for (const auto& r : rules_list)
    {
        const auto eligible =
            rules::router_profile::GetEligible(
                r.type
            );

        const auto skipped =
            rules::router_profile::GetSkipped(
                r.type
            );

        const auto called =
            rules::router_profile::GetCalled(
                r.type
            );

        const auto matched =
            rules::router_profile::GetMatched(
                r.type
            );

        eligible_total += eligible;
        skipped_total += skipped;
        called_total += called;
        matched_total += matched;

        std::cout
            << std::left
            << std::setw(14)
            << r.name
            << std::right
            << std::setw(13)
            << eligible
            << std::setw(13)
            << skipped
            << std::setw(13)
            << called
            << std::setw(13)
            << matched
            << '\n';
    }

    const auto decisions =
        called_total +
        skipped_total;

    const double skip_rate =
        decisions
        ? 100.0 *
          static_cast<double>(skipped_total) /
          static_cast<double>(decisions)
        : 0.0;

    const double match_rate =
        called_total
        ? 100.0 *
          static_cast<double>(matched_total) /
          static_cast<double>(called_total)
        : 0.0;

    std::cout
        << std::string(66, '-')
        << '\n'
        << std::left
        << std::setw(14)
        << "TOTAL"
        << std::right
        << std::setw(13)
        << eligible_total
        << std::setw(13)
        << skipped_total
        << std::setw(13)
        << called_total
        << std::setw(13)
        << matched_total
        << "\n\n"
        << std::fixed
        << std::setprecision(2)
        << "Router skip rate : "
        << skip_rate
        << "%\n"
        << "Called match rate: "
        << match_rate
        << "%\n"
        << "Output spans     : "
        << result.Value().size()
        << "\n\n"
        << "[PASS] #96 COMPLETE\n"
        << "============================================================\n";

    return 0;
}
