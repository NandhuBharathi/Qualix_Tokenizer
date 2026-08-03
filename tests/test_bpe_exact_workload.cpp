#include "rules/numeric_scanner.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

#include "bpe/span_encoder.hpp"
#include "bpe/vocabulary.hpp"
#include "pretokenizer/pretokenizer.hpp"

using namespace qualix;
using namespace qualix::bpe;
using namespace qualix::pretokenizer;

using Clock = std::chrono::steady_clock;

static double Seconds(
    Clock::time_point a,
    Clock::time_point b
)
{
    return std::chrono::duration<double>(
        b - a
    ).count();
}

static std::string BuildCorpus()
{
    const std::string sample =
        "Hello world! "
        "தமிழ் மொழி அழகானது. "
        "Email test@example.com "
        "URL https://example.com/path?q=1 "
        "Date 2026-08-02 "
        "Time 10:30 PM "
        "Price ₹1,250.50 "
        "Percent 98.5% "
        "Measure 12.5kg "
        "Phone +91 98765 43210 "
        "Math x^2 + y^2 = z^2 "
        "Unicode café naïve résumé "
        "Emoji 😀🔥❤️ "
        "1234567890 abcdefghijklmnopqrstuvwxyz\n";

    std::string corpus;

    /*
     * Reproduce approximately the same workload
     * used by the earlier ~0.196 MB pipeline test.
     */
    while (corpus.size() < 200000)
        corpus += sample;

    return corpus;
}

int main()
{
    std::cout
        << "============================================================\n"
        << "QUALIX — #88 EXACT WORKLOAD REPRODUCTION\n"
        << "============================================================\n";

    const std::string corpus =
        BuildCorpus();

    const double mb =
        static_cast<double>(corpus.size()) /
        (1024.0 * 1024.0);

    std::cout
        << std::fixed
        << std::setprecision(3)
        << "Corpus : "
        << mb
        << " MB\n\n";

    /*
     * ---------------------------------------------------------
     * 1. Direct PreTokenizer
     * ---------------------------------------------------------
     */
    const auto p0 = Clock::now();

    auto split =
        PreTokenizer::Split(
            corpus
        );

    const auto p1 = Clock::now();

    if (split.Failed())
    {
        std::cerr
            << "[FAIL] PreTokenizer\n";

        return 1;
    }

    const double pretokenizer_time =
        Seconds(p0, p1);

    /*
     * ---------------------------------------------------------
     * 2. Full SpanEncoder
     * ---------------------------------------------------------
     */
    Vocabulary vocabulary;

    const auto s0 = Clock::now();

    auto encoded =
        BpeSpanEncoder::Encode(
            corpus,
            vocabulary
        );

    const auto s1 = Clock::now();

    if (encoded.Failed())
    {
        std::cerr
            << "[FAIL] SpanEncoder\n";

        return 1;
    }

    const double span_time =
        Seconds(s0, s1);

    /*
     * ---------------------------------------------------------
     * Result
     * ---------------------------------------------------------
     */
    std::cout
        << "Direct PreTokenizer\n"
        << "-------------------\n"
        << "Time       : "
        << pretokenizer_time
        << " s\n"
        << "Throughput : "
        << (
            pretokenizer_time > 0.0
                ? mb / pretokenizer_time
                : 0.0
        )
        << " MB/s\n"
        << "Spans      : "
        << split.Value().size()
        << "\n\n";

    std::cout
        << "BpeSpanEncoder\n"
        << "--------------\n"
        << "Time       : "
        << span_time
        << " s\n"
        << "Throughput : "
        << (
            span_time > 0.0
                ? mb / span_time
                : 0.0
        )
        << " MB/s\n"
        << "Spans      : "
        << encoded.Value().size()
        << "\n"
        << "Vocabulary : "
        << vocabulary.Size()
        << "\n\n";

    const double overhead =
        span_time -
        pretokenizer_time;

    std::cout
        << "Difference\n"
        << "----------\n"
        << "SpanEncoder overhead : "
        << overhead
        << " s\n";

    if (pretokenizer_time > 0.0)
    {
        std::cout
            << "Span/PreTokenizer    : "
            << span_time /
               pretokenizer_time
            << "x\n";
    }

    std::cout
        << "============================================================\n";

    qualix::rules::DumpNumericScanProfile();
    return 0;
}
