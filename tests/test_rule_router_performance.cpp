
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

#include "pretokenizer/pretokenizer.hpp"

using namespace qualix;
using namespace qualix::pretokenizer;

static std::string BuildCorpus(std::size_t target)
{
    const std::string sample =
        "Hello world this is Qualix tokenizer test. "
        "Price Rs.500 and USD 500 and EUR 1,000.50. "
        "Email test@example.com website https://example.com/path. "
        "Date 2026-08-02 time 10:30 AM phone +91 9876543210. "
        "Value 75% distance 25 km equation x+10=20. "
        "Tamil தமிழ் மொழி tokenizer performance test. ";

    std::string corpus;
    corpus.reserve(target + sample.size());

    while (corpus.size() < target)
        corpus += sample;

    corpus.resize(target);
    return corpus;
}

int main()
{
    constexpr std::size_t target =
        10ULL * 1024ULL * 1024ULL;

    const std::string corpus =
        BuildCorpus(target);

    // Warm-up
    {
        auto warm =
            PreTokenizer::Split(
                std::string_view(
                    corpus.data(),
                    std::min<std::size_t>(
                        corpus.size(),
                        64 * 1024
                    )
                )
            );

        if (warm.Failed())
        {
            std::cout << "[FAIL] WARMUP\n";
            return 1;
        }
    }

    const auto start =
        std::chrono::steady_clock::now();

    auto result =
        PreTokenizer::Split(corpus);

    const auto end =
        std::chrono::steady_clock::now();

    if (result.Failed())
    {
        std::cout << "[FAIL] PRETOKENIZER\n";
        return 1;
    }

    const double seconds =
        std::chrono::duration<double>(
            end - start
        ).count();

    const double mb =
        static_cast<double>(
            corpus.size()
        ) / (1024.0 * 1024.0);

    const double rate =
        mb / seconds;

    constexpr double baseline =
        1.166;

    const double speedup =
        rate / baseline;

    std::cout
        << std::fixed
        << std::setprecision(3);

    std::cout
        << "Corpus       : "
        << mb << " MB\n";

    std::cout
        << "Time         : "
        << seconds << " s\n";

    std::cout
        << "Throughput   : "
        << rate << " MB/s\n";

    std::cout
        << "Baseline     : "
        << baseline << " MB/s\n";

    std::cout
        << "Speedup      : "
        << speedup << "x\n";

    std::cout
        << "Output spans : "
        << result.Value().size()
        << '\n';

    return 0;
}
