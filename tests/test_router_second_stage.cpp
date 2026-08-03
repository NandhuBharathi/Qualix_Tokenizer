
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

#include "pretokenizer/pretokenizer.hpp"

using namespace qualix;

namespace
{

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
    corpus.reserve(
        bytes + block.size()
    );

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

    /*
     * Warm-up prevents first-run effects from
     * dominating the measurement.
     */
    {
        const std::string warm =
            corpus.substr(
                0,
                64 * 1024
            );

        auto r =
            pretokenizer::PreTokenizer::Split(
                warm
            );

        if (r.Failed())
            return 1;
    }

    constexpr int runs = 3;

    double best = 1e100;
    std::size_t spans = 0;

    for (int i = 0; i < runs; ++i)
    {
        const auto start =
            std::chrono::steady_clock::now();

        auto result =
            pretokenizer::PreTokenizer::Split(
                corpus
            );

        const auto end =
            std::chrono::steady_clock::now();

        if (result.Failed())
            return 2;

        const double seconds =
            std::chrono::duration<double>(
                end - start
            ).count();

        if (seconds < best)
            best = seconds;

        spans =
            result.Value().size();
    }

    const double mb =
        static_cast<double>(bytes) /
        (1024.0 * 1024.0);

    std::cout
        << std::fixed
        << std::setprecision(3)
        << best << "\n"
        << (mb / best) << "\n"
        << spans << "\n";

    return 0;
}
