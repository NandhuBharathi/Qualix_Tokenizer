
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

#include "unicode/grapheme_segmenter.hpp"

using namespace qualix;

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

    using Clock = std::chrono::steady_clock;

    constexpr int Runs = 3;

    double total = 0.0;
    usize grapheme_count = 0;

    std::cout
        << "\n============================================================\n"
        << "QUALIX — #86 GRAPHEME SEGMENTER BENCHMARK\n"
        << "============================================================\n"
        << "Corpus : "
        << std::fixed << std::setprecision(3)
        << corpus.size() / 1000000.0
        << " MB\n\n";

    for (int run = 1; run <= Runs; ++run)
    {
        const auto start = Clock::now();

        auto result =
            unicode::GraphemeSegmenter::Segment(
                corpus
            );

        const auto end = Clock::now();

        if (result.Failed())
        {
            std::cout << "[FAIL] SEGMENT\n";
            return 1;
        }

        const double seconds =
            std::chrono::duration<double>(
                end - start
            ).count();

        grapheme_count =
            result.Value().size();

        total += seconds;

        const double rate =
            seconds > 0.0
                ? (corpus.size() / 1000000.0) /
                    seconds
                : 0.0;

        std::cout
            << "Run " << run
            << " : "
            << std::fixed
            << std::setprecision(3)
            << seconds
            << " s | "
            << rate
            << " MB/s\n";
    }

    const double average =
        total / Runs;

    std::cout
        << "\nGraphemes : "
        << grapheme_count
        << "\nAverage   : "
        << std::fixed
        << std::setprecision(3)
        << average
        << " s\n"
        << "Throughput: "
        << (corpus.size() / 1000000.0) /
            average
        << " MB/s\n"
        << "============================================================\n";

    return 0;
}
