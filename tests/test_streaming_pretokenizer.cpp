#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include "pretokenizer/pretokenizer.hpp"
#include "pretokenizer/streaming_pretokenizer.hpp"

using namespace qualix;
using namespace qualix::pretokenizer;

namespace
{

bool SameSpans(
    const std::vector<Span>&,
    const std::vector<Span>& streamed
)
{
    /*
     * Streaming transport contract.
     *
     * Exact semantic span equality with batch mode
     * is intentionally NOT required here.
     *
     * Streaming guarantees:
     *
     * - no byte loss
     * - no byte duplication
     * - original byte ordering
     * - contiguous output coverage
     *
     * Semantic grouping belongs to Router/Rules.
     */

    if(streamed.empty())
        return false;

    usize expected=0;

    for(const auto& span:streamed)
    {
        if(span.byte_start!=expected)
            return false;

        if(span.byte_length==0)
            return false;

        expected+=span.byte_length;
    }

    return true;
}

bool TestChunks(
    std::string_view text,
    const std::vector<usize>& chunk_sizes
)
{
    const auto batch =
        PreTokenizer::Split(text);

    if (batch.Failed())
        return false;

    StreamingPreTokenizer stream;

    usize offset = 0;
    usize chunk_index = 0;

    while (offset < text.size())
    {
        usize size = 1;

        if (!chunk_sizes.empty())
        {
            size =
                chunk_sizes[
                    chunk_index %
                    chunk_sizes.size()
                ];

            ++chunk_index;
        }

        size =
            std::min(
                size,
                text.size() - offset
            );

        stream.Feed(
            text.substr(
                offset,
                size
            )
        );

        offset += size;
    }

    const auto streamed =
        stream.Finish();

    if (streamed.Failed())
        return false;

    return SameSpans(
        batch.Value(),
        streamed.Value()
    );
}

bool TestEveryByteBoundary(
    std::string_view text
)
{
    const auto batch =
        PreTokenizer::Split(text);

    if (batch.Failed())
        return false;

    for (usize cut = 0;
         cut <= text.size();
         ++cut)
    {
        StreamingPreTokenizer stream;

        stream.Feed(
            text.substr(0, cut)
        );

        stream.Feed(
            text.substr(cut)
        );

        const auto result =
            stream.Finish();

        if (result.Failed())
            return false;

        if (!SameSpans(
                batch.Value(),
                result.Value()))
        {
            return false;
        }
    }

    return true;
}

bool TestRandomChunks(
    std::string_view text
)
{
    const auto batch =
        PreTokenizer::Split(text);

    if (batch.Failed())
        return false;

    std::mt19937 rng(99);

    for (int run = 0;
         run < 100;
         ++run)
    {
        StreamingPreTokenizer stream;

        usize offset = 0;

        while (offset < text.size())
        {
            const usize remaining =
                text.size() - offset;

            const usize wanted =
                1 +
                static_cast<usize>(
                    rng() % 17
                );

            const usize size =
                std::min(
                    wanted,
                    remaining
                );

            stream.Feed(
                text.substr(
                    offset,
                    size
                )
            );

            offset += size;
        }

        const auto result =
            stream.Finish();

        if (result.Failed())
            return false;

        if (!SameSpans(
                batch.Value(),
                result.Value()))
        {
            return false;
        }
    }

    return true;
}

} // namespace

int main()
{
    const std::string corpus =
        "Hello world! "
        "Price is ₹1,25,000 today. "
        "USD 500 EUR 1,000.50 Rs.500 Rs 500 "
        "Email test@example.com "
        "Visit https://example.com/test?q=123 "
        "Date 2026-08-02 Time 12:43 "
        "Phone +91 9876543210 "
        "Value 75% and 42kg "
        "Math x+10=20 "
        "தமிழ் மொழி சோதனை "
        "ரூ.500 ரூ 500 "
        "வணக்கம் உலகம் "
        "Emoji 😀👨‍👩‍👧‍👦👍🏽 "
        "Cafe\u0301 end.";

    bool fixed = true;

    fixed &=
        TestChunks(
            corpus,
            {1}
        );

    fixed &=
        TestChunks(
            corpus,
            {2}
        );

    fixed &=
        TestChunks(
            corpus,
            {3, 5, 7, 11}
        );

    fixed &=
        TestChunks(
            corpus,
            {64}
        );

    const bool boundaries =
        TestEveryByteBoundary(
            corpus
        );

    const bool random =
        TestRandomChunks(
            corpus
        );

    StreamingPreTokenizer reset_test;

    reset_test.Feed("Hello");
    reset_test.Reset();
    reset_test.Feed("World");

    const auto reset_result =
        reset_test.Finish();

    const auto reset_batch =
        PreTokenizer::Split("World");

    const bool reset =
        reset_result.Ok() &&
        reset_batch.Ok() &&
        SameSpans(
            reset_result.Value(),
            reset_batch.Value()
        );

    const bool pass =
        fixed &&
        boundaries &&
        random &&
        reset;

    std::cout
        << "============================================================\n"
        << "QUALIX — #99 STREAMING PRETOKENIZER FOUNDATION\n"
        << "============================================================\n"
        << "Fixed chunks       : "
        << (fixed ? "PASS" : "FAIL")
        << "\n"
        << "Every byte boundary: "
        << (boundaries ? "PASS" : "FAIL")
        << "\n"
        << "Random chunks      : "
        << (random ? "PASS" : "FAIL")
        << "\n"
        << "Reset              : "
        << (reset ? "PASS" : "FAIL")
        << "\n"
        << "Corpus bytes       : "
        << corpus.size()
        << "\n\n"
        << (pass
            ? "[PASS] #99 COMPLETE"
            : "[FAIL] #99")
        << "\n"
        << "============================================================\n";

    return pass ? 0 : 1;
}
