
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "unicode/grapheme_segmenter.hpp"
#include "pretokenizer/pretokenizer.hpp"

using namespace qualix;

namespace
{

using Clock =
    std::chrono::steady_clock;

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

    return corpus;
}

std::vector<std::string_view>
MakeChunks(
    const std::string& corpus,
    std::size_t count
)
{
    std::vector<std::string_view> out;

    out.reserve(count);

    std::size_t start = 0;

    for (std::size_t i = 0;
         i < count;
         ++i)
    {
        if (i + 1 == count)
        {
            out.emplace_back(
                corpus.data() + start,
                corpus.size() - start
            );

            break;
        }

        const std::size_t remaining =
            corpus.size() - start;

        const std::size_t workers =
            count - i;

        std::size_t end =
            start +
            remaining / workers;

        while (end < corpus.size() &&
               corpus[end] != '\n')
        {
            ++end;
        }

        if (end < corpus.size())
            ++end;

        out.emplace_back(
            corpus.data() + start,
            end - start
        );

        start = end;
    }

    return out;
}

struct Result
{
    double seconds = 0.0;
    std::size_t items = 0;
    bool ok = true;
};

template <typename Function>
Result ParallelRun(
    const std::vector<std::string_view>& chunks,
    Function function
)
{
    std::vector<std::thread> workers;

    std::vector<std::size_t> counts(
        chunks.size(),
        0
    );

    std::vector<unsigned char> success(
        chunks.size(),
        0
    );

    workers.reserve(
        chunks.size()
    );

    const auto begin =
        Clock::now();

    for (std::size_t i = 0;
         i < chunks.size();
         ++i)
    {
        workers.emplace_back(
            [&, i]()
            {
                const auto result =
                    function(chunks[i]);

                if (!result.first)
                    return;

                counts[i] =
                    result.second;

                success[i] = 1;
            }
        );
    }

    for (auto& worker : workers)
        worker.join();

    const auto end =
        Clock::now();

    Result result;

    result.seconds =
        std::chrono::duration<double>(
            end - begin
        ).count();

    for (std::size_t i = 0;
         i < chunks.size();
         ++i)
    {
        if (!success[i])
        {
            result.ok = false;
            return result;
        }

        result.items +=
            counts[i];
    }

    return result;
}

Result GraphemeBench(
    const std::vector<std::string_view>& chunks
)
{
    return ParallelRun(
        chunks,
        [](std::string_view input)
        {
            auto result =
                unicode::GraphemeSegmenter::Segment(
                    input
                );

            if (result.Failed())
            {
                return std::pair{
                    false,
                    std::size_t{0}
                };
            }

            return std::pair{
                true,
                result.Value().size()
            };
        }
    );
}

Result PretokenizerBench(
    const std::vector<std::string_view>& chunks
)
{
    return ParallelRun(
        chunks,
        [](std::string_view input)
        {
            auto result =
                pretokenizer::PreTokenizer::Split(
                    input
                );

            if (result.Failed())
            {
                return std::pair{
                    false,
                    std::size_t{0}
                };
            }

            return std::pair{
                true,
                result.Value().size()
            };
        }
    );
}

double Rate(
    double mb,
    double seconds
)
{
    return
        seconds > 0.0
        ? mb / seconds
        : 0.0;
}

} // namespace

int main()
{
    constexpr std::size_t target =
        10ULL * 1024ULL * 1024ULL;

    const std::string corpus =
        BuildCorpus(target);

    const double mb =
        static_cast<double>(
            corpus.size()
        ) /
        (1024.0 * 1024.0);

    /*
     * Warm-up.
     */
    {
        const std::size_t size =
            std::min<std::size_t>(
                corpus.size(),
                64 * 1024
            );

        auto warm =
            pretokenizer::PreTokenizer::Split(
                std::string_view{
                    corpus.data(),
                    size
                }
            );

        if (warm.Failed())
            return 1;
    }

    const auto chunks1 =
        MakeChunks(corpus, 1);

    const auto chunks2 =
        MakeChunks(corpus, 2);

    const auto chunks4 =
        MakeChunks(corpus, 4);

    const auto g1 =
        GraphemeBench(chunks1);

    const auto g2 =
        GraphemeBench(chunks2);

    const auto g4 =
        GraphemeBench(chunks4);

    const auto p1 =
        PretokenizerBench(chunks1);

    const auto p2 =
        PretokenizerBench(chunks2);

    const auto p4 =
        PretokenizerBench(chunks4);

    if (!g1.ok ||
        !g2.ok ||
        !g4.ok ||
        !p1.ok ||
        !p2.ok ||
        !p4.ok)
    {
        return 1;
    }

    if (g1.items != g2.items ||
        g1.items != g4.items ||
        p1.items != p2.items ||
        p1.items != p4.items)
    {
        return 2;
    }

    std::cout
        << std::fixed
        << std::setprecision(3);

    std::cout
        << "Corpus        : "
        << mb
        << " MB\n";

    std::cout
        << "CPU HW        : "
        << std::thread::hardware_concurrency()
        << " threads\n\n";

    const double g1rate =
        Rate(mb, g1.seconds);

    const double g2rate =
        Rate(mb, g2.seconds);

    const double g4rate =
        Rate(mb, g4.seconds);

    std::cout
        << "GRAPHEME\n";

    std::cout
        << "1 Thread      : "
        << g1.seconds << " s | "
        << g1rate << " MB/s | 1.000x\n";

    std::cout
        << "2 Threads     : "
        << g2.seconds << " s | "
        << g2rate << " MB/s | "
        << g2rate / g1rate << "x\n";

    std::cout
        << "4 Threads     : "
        << g4.seconds << " s | "
        << g4rate << " MB/s | "
        << g4rate / g1rate << "x\n\n";

    const double p1rate =
        Rate(mb, p1.seconds);

    const double p2rate =
        Rate(mb, p2.seconds);

    const double p4rate =
        Rate(mb, p4.seconds);

    std::cout
        << "PRETOKENIZER\n";

    std::cout
        << "1 Thread      : "
        << p1.seconds << " s | "
        << p1rate << " MB/s | 1.000x\n";

    std::cout
        << "2 Threads     : "
        << p2.seconds << " s | "
        << p2rate << " MB/s | "
        << p2rate / p1rate << "x\n";

    std::cout
        << "4 Threads     : "
        << p4.seconds << " s | "
        << p4rate << " MB/s | "
        << p4rate / p1rate << "x\n\n";

    /*
     * Approximate non-grapheme part.
     *
     * Not an independent timer: this is
     * PreTokenizer wall time minus Grapheme
     * wall time for the same thread count.
     */
    const double rest1 =
        std::max(
            0.0,
            p1.seconds - g1.seconds
        );

    const double rest2 =
        std::max(
            0.0,
            p2.seconds - g2.seconds
        );

    const double rest4 =
        std::max(
            0.0,
            p4.seconds - g4.seconds
        );

    std::cout
        << "APPROX NON-GRAPHEME\n";

    std::cout
        << "1 Thread      : "
        << rest1 << " s\n";

    std::cout
        << "2 Threads     : "
        << rest2 << " s\n";

    std::cout
        << "4 Threads     : "
        << rest4 << " s\n";

    return 0;
}
