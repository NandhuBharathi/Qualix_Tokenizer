
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

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

static std::string BuildCorpus(
    usize target_bytes
)
{
    const std::string sample =
        "Hello world! "
        "தமிழ் மொழி அழகானது. "
        "English and Tamil tokenizer benchmark. "
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

    corpus.reserve(
        target_bytes + sample.size()
    );

    while (corpus.size() < target_bytes)
        corpus += sample;

    corpus.resize(target_bytes);

    return corpus;
}

struct TestSize
{
    const char* name;
    usize bytes;
};

int main()
{
    const usize MiB =
        1024 * 1024;

    const std::vector<TestSize> sizes{
        {
            "0.25 MB",
            MiB / 4
        },
        {
            "1 MB",
            MiB
        },
        {
            "5 MB",
            5 * MiB
        },
        {
            "10 MB",
            10 * MiB
        }
    };

    std::cout
        << "============================================================\n"
        << "QUALIX — #89 SCALING BENCHMARK\n"
        << "============================================================\n\n";

    std::cout
        << std::fixed
        << std::setprecision(3);

    for (const auto& size : sizes)
    {
        std::cout
            << "------------------------------------------------------------\n"
            << "TARGET : "
            << size.name
            << "\n"
            << "------------------------------------------------------------\n";

        const std::string corpus =
            BuildCorpus(
                size.bytes
            );

        const double mb =
            static_cast<double>(
                corpus.size()
            ) /
            static_cast<double>(MiB);

        /*
         * Direct PreTokenizer.
         */
        const auto p0 =
            Clock::now();

        auto split =
            PreTokenizer::Split(
                corpus
            );

        const auto p1 =
            Clock::now();

        if (split.Failed())
        {
            std::cerr
                << "[FAIL] PreTokenizer at "
                << size.name
                << '\n';

            return 1;
        }

        const double pre_time =
            Seconds(p0, p1);

        /*
         * Full BPE SpanEncoder.
         *
         * Fresh vocabulary for every size so
         * benchmark runs remain independent.
         */
        Vocabulary vocabulary;

        const auto s0 =
            Clock::now();

        auto encoded =
            BpeSpanEncoder::Encode(
                corpus,
                vocabulary
            );

        const auto s1 =
            Clock::now();

        if (encoded.Failed())
        {
            std::cerr
                << "[FAIL] SpanEncoder at "
                << size.name
                << '\n';

            return 1;
        }

        const double span_time =
            Seconds(s0, s1);

        const double pre_rate =
            pre_time > 0.0
                ? mb / pre_time
                : 0.0;

        const double span_rate =
            span_time > 0.0
                ? mb / span_time
                : 0.0;

        const double overhead =
            span_time - pre_time;

        std::cout
            << "Corpus              : "
            << mb
            << " MB\n"

            << "PreTokenizer        : "
            << pre_time
            << " s\n"

            << "PreTokenizer rate   : "
            << pre_rate
            << " MB/s\n"

            << "SpanEncoder         : "
            << span_time
            << " s\n"

            << "SpanEncoder rate    : "
            << span_rate
            << " MB/s\n"

            << "Span overhead       : "
            << overhead
            << " s\n"

            << "Spans               : "
            << split.Value().size()
            << '\n'

            << "Encoded spans       : "
            << encoded.Value().size()
            << '\n'

            << "Vocabulary          : "
            << vocabulary.Size()
            << '\n';

        if (pre_time > 0.0)
        {
            std::cout
                << "Span/Pre ratio      : "
                << span_time / pre_time
                << "x\n";
        }

        std::cout << '\n';
    }

    std::cout
        << "============================================================\n"
        << "[PASS] #89 SCALING BENCHMARK COMPLETE\n"
        << "============================================================\n";

    return 0;
}
