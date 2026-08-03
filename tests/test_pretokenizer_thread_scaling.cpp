
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "pretokenizer/pretokenizer.hpp"

using namespace qualix;
using namespace qualix::pretokenizer;

namespace
{

std::string BuildCorpus(
    std::size_t target
)
{
    const std::string sample =
        "Hello world Qualix tokenizer test. "
        "Price Rs.500 USD 500 EUR 1,000.50. "
        "Email test@example.com "
        "URL https://example.com/path "
        "Date 2026-08-02 Time 10:30 AM "
        "Phone +91 9876543210 "
        "Value 75% distance 25 km "
        "equation x+10=20. "
        "Tamil தமிழ் மொழி tokenizer test.\n";

    std::string corpus;
    corpus.reserve(
        target + sample.size()
    );

    while (corpus.size() < target)
        corpus += sample;

    /*
     * Do not cut the final sample in half.
     *
     * This keeps benchmark chunks easier to
     * split safely at newline boundaries.
     */
    return corpus;
}

struct BenchResult
{
    double seconds = 0.0;
    double rate = 0.0;
    std::size_t spans = 0;
    bool ok = true;
};

std::vector<std::string_view>
MakeChunks(
    const std::string& corpus,
    std::size_t thread_count
)
{
    std::vector<std::string_view> chunks;

    chunks.reserve(thread_count);

    std::size_t start = 0;

    for (std::size_t i = 0;
         i < thread_count;
         ++i)
    {
        if (i + 1 == thread_count)
        {
            chunks.emplace_back(
                corpus.data() + start,
                corpus.size() - start
            );

            break;
        }

        const std::size_t remaining_threads =
            thread_count - i;

        const std::size_t remaining_bytes =
            corpus.size() - start;

        std::size_t target =
            start +
            remaining_bytes /
            remaining_threads;

        /*
         * Move forward to a newline.
         *
         * Therefore we never split a URL,
         * email, currency expression, UTF-8
         * sequence, grapheme, etc. in the
         * middle for this benchmark corpus.
         */
        while (target < corpus.size() &&
               corpus[target] != '\n')
        {
            ++target;
        }

        if (target < corpus.size())
            ++target;

        chunks.emplace_back(
            corpus.data() + start,
            target - start
        );

        start = target;
    }

    return chunks;
}

BenchResult Run(
    const std::string& corpus,
    std::size_t thread_count
)
{
    const auto chunks =
        MakeChunks(
            corpus,
            thread_count
        );

    std::vector<std::size_t> span_counts(
        chunks.size(),
        0
    );

    std::vector<bool> success(
        chunks.size(),
        false
    );

    std::vector<std::thread> workers;

    workers.reserve(
        chunks.size()
    );

    const auto begin =
        std::chrono::steady_clock::now();

    for (std::size_t i = 0;
         i < chunks.size();
         ++i)
    {
        workers.emplace_back(
            [&, i]()
            {
                auto result =
                    PreTokenizer::Split(
                        chunks[i]
                    );

                if (result.Failed())
                    return;

                span_counts[i] =
                    result.Value().size();

                success[i] = true;
            }
        );
    }

    for (auto& worker : workers)
        worker.join();

    const auto end =
        std::chrono::steady_clock::now();

    BenchResult output;

    for (std::size_t i = 0;
         i < success.size();
         ++i)
    {
        if (!success[i])
        {
            output.ok = false;
            return output;
        }

        output.spans +=
            span_counts[i];
    }

    output.seconds =
        std::chrono::duration<double>(
            end - begin
        ).count();

    const double mb =
        static_cast<double>(
            corpus.size()
        ) /
        (1024.0 * 1024.0);

    output.rate =
        mb / output.seconds;

    return output;
}

} // namespace

int main()
{
    constexpr std::size_t target =
        10ULL *
        1024ULL *
        1024ULL;

    const std::string corpus =
        BuildCorpus(target);

    /*
     * Warm-up outside benchmark.
     */
    {
        const std::size_t warm_size =
            std::min<std::size_t>(
                corpus.size(),
                64 * 1024
            );

        auto warm =
            PreTokenizer::Split(
                std::string_view{
                    corpus.data(),
                    warm_size
                }
            );

        if (warm.Failed())
            return 1;
    }

    const auto one =
        Run(corpus, 1);

    const auto two =
        Run(corpus, 2);

    const auto four =
        Run(corpus, 4);

    if (!one.ok ||
        !two.ok ||
        !four.ok)
    {
        return 1;
    }

    /*
     * Safe chunking should preserve total
     * span count regardless of worker count.
     */
    if (one.spans != two.spans ||
        one.spans != four.spans)
    {
        return 2;
    }

    const double mb =
        static_cast<double>(
            corpus.size()
        ) /
        (1024.0 * 1024.0);

    std::cout
        << std::fixed
        << std::setprecision(3);

    std::cout
        << "Corpus     : "
        << mb
        << " MB\n";

    std::cout
        << "CPU HW     : "
        << std::thread::hardware_concurrency()
        << " threads\n";

    std::cout
        << '\n';

    std::cout
        << "1 Thread   : "
        << one.seconds
        << " s | "
        << one.rate
        << " MB/s | 1.000x\n";

    std::cout
        << "2 Threads  : "
        << two.seconds
        << " s | "
        << two.rate
        << " MB/s | "
        << two.rate / one.rate
        << "x\n";

    std::cout
        << "4 Threads  : "
        << four.seconds
        << " s | "
        << four.rate
        << " MB/s | "
        << four.rate / one.rate
        << "x\n";

    std::cout
        << '\n';

    std::cout
        << "Spans      : "
        << one.spans
        << '\n';

    return 0;
}
