#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "pretokenizer/pretokenizer.hpp"
#include "pretokenizer/streaming_pretokenizer.hpp"

using namespace qualix;
using namespace qualix::pretokenizer;

namespace
{

const char* TypeName(
    SpanType type
)
{
    switch (type)
    {
        case SpanType::Unknown:     return "Unknown";
        case SpanType::Word:        return "Word";
        case SpanType::Number:      return "Number";
        case SpanType::Whitespace:  return "Whitespace";
        case SpanType::Punctuation: return "Punctuation";
        case SpanType::Symbol:      return "Symbol";
        case SpanType::Emoji:       return "Emoji";
        case SpanType::Url:         return "Url";
        case SpanType::Email:       return "Email";
        case SpanType::Date:        return "Date";
        case SpanType::Time:        return "Time";
        case SpanType::Phone:       return "Phone";
        case SpanType::Currency:    return "Currency";
        case SpanType::Percentage:  return "Percentage";
        case SpanType::Measurement: return "Measurement";
        case SpanType::Math:        return "Math";
        case SpanType::Code:        return "Code";
        case SpanType::Other:       return "Other";
    }

    return "?";
}

bool Same(
    const Span& a,
    const Span& b
)
{
    return
        a.byte_start == b.byte_start &&
        a.byte_length == b.byte_length &&
        a.grapheme_start == b.grapheme_start &&
        a.grapheme_count == b.grapheme_count &&
        a.type == b.type &&
        a.policy == b.policy;
}

void PrintSpan(
    const char* label,
    const Span& span,
    std::string_view text
)
{
    std::cout
        << label
        << " type=" << TypeName(span.type)
        << " byte=[" << span.byte_start
        << "," << span.ByteEnd() << ")"
        << " grapheme=[" << span.grapheme_start
        << "," << span.GraphemeEnd() << ")"
        << " protected="
        << (span.Protected() ? "yes" : "no")
        << " text=["
        << span.View(text)
        << "]\n";
}

bool Probe(
    std::string_view text,
    usize chunk_size
)
{
    const auto batch =
        PreTokenizer::Split(text);

    if (batch.Failed())
    {
        std::cout << "[FAIL] Batch tokenizer\n";
        return false;
    }

    StreamingPreTokenizer stream;

    usize offset = 0;

    while (offset < text.size())
    {
        const usize count =
            std::min(
                chunk_size,
                text.size() - offset
            );

        stream.Feed(
            text.substr(
                offset,
                count
            )
        );

        offset += count;
    }

    const auto streaming =
        stream.Finish();

    if (streaming.Failed())
    {
        std::cout << "[FAIL] Streaming tokenizer\n";
        return false;
    }

    const auto& a = batch.Value();
    const auto& b = streaming.Value();

    const usize common =
        std::min(
            a.size(),
            b.size()
        );

    for (usize i = 0;
         i < common;
         ++i)
    {
        if (!Same(a[i], b[i]))
        {
            std::cout
                << "Chunk size     : "
                << chunk_size
                << "\n"
                << "Mismatch index : "
                << i
                << "\n"
                << "Batch spans    : "
                << a.size()
                << "\n"
                << "Stream spans   : "
                << b.size()
                << "\n\n";

            PrintSpan(
                "BATCH ",
                a[i],
                text
            );

            PrintSpan(
                "STREAM",
                b[i],
                text
            );

            if (i > 0)
            {
                std::cout << "\nPrevious:\n";

                PrintSpan(
                    "BATCH ",
                    a[i - 1],
                    text
                );

                PrintSpan(
                    "STREAM",
                    b[i - 1],
                    text
                );
            }

            return false;
        }
    }

    if (a.size() != b.size())
    {
        std::cout
            << "Chunk size     : "
            << chunk_size
            << "\n"
            << "Span count mismatch\n"
            << "Batch spans    : "
            << a.size()
            << "\n"
            << "Stream spans   : "
            << b.size()
            << "\n";

        return false;
    }

    return true;
}

}

int main()
{
    const std::string text =
        "Hello தமிழ் world "
        "Price ₹1,25,000 "
        "USD 500 "
        "EUR 1,000.50 "
        "mail test@example.com "
        "url https://example.com/path?q=123 "
        "date 2026-08-02 "
        "time 12:48:59 "
        "phone +91 9876543210 "
        "value 75% "
        "weight 42kg "
        "emoji 😀👨‍👩‍👧‍👦👍🏽 "
        "Cafe\u0301 end\n";

    const usize chunks[] =
    {
        1, 2, 3, 4, 5, 7,
        8, 11, 16, 31, 64
    };

    for (const usize chunk : chunks)
    {
        if (!Probe(text, chunk))
        {
            std::cout
                << "\n[FAIL] #101.1 MISMATCH FOUND\n";

            return 1;
        }
    }

    std::cout
        << "[PASS] NO MISMATCH FOUND\n";

    return 0;
}
