#include <string>
#include <vector>

#include "pretokenizer/pretokenizer.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::pretokenizer;
using namespace qualix::test;

struct ExpectedSpan
{
    std::string text;
    SpanType type;
    usize grapheme_count;
};

static void Check(
    const std::string& input,
    const std::vector<ExpectedSpan>& expected,
    const std::string& name
)
{
    auto result =
        PreTokenizer::Split(input);

    Expect(
        result.Ok(),
        name + " split"
    );

    if (result.Failed())
        return;

    const auto& spans =
        result.Value();

    Expect(
        spans.size() == expected.size(),
        name + " span count"
    );

    if (spans.size() != expected.size())
        return;

    usize expected_byte_start = 0;
    usize expected_grapheme_start = 0;

    for (usize i = 0;
         i < spans.size();
         ++i)
    {
        const auto& span = spans[i];

        Expect(
            span.View(input) ==
                expected[i].text,
            name +
                " text " +
                std::to_string(i)
        );

        Expect(
            span.type ==
                expected[i].type,
            name +
                " type " +
                std::to_string(i)
        );

        Expect(
            span.grapheme_count ==
                expected[i].grapheme_count,
            name +
                " grapheme count " +
                std::to_string(i)
        );

        Expect(
            span.byte_start ==
                expected_byte_start,
            name +
                " byte continuity " +
                std::to_string(i)
        );

        Expect(
            span.grapheme_start ==
                expected_grapheme_start,
            name +
                " grapheme continuity " +
                std::to_string(i)
        );

        expected_byte_start +=
            span.byte_length;

        expected_grapheme_start +=
            span.grapheme_count;
    }

    Expect(
        expected_byte_start ==
            input.size(),
        name + " covers all bytes"
    );
}

int main()
{
    Check(
        "Hello",
        {
            {"Hello", SpanType::Word, 5}
        },
        "ASCII word"
    );

    Check(
        "தமிழ்",
        {
            {"தமிழ்", SpanType::Word, 3}
        },
        "Tamil word"
    );

    Check(
        "नमस्ते",
        {
            {"नमस्ते", SpanType::Word, 3}
        },
        "Devanagari word"
    );

    Check(
        "中文测试",
        {
            {"中文测试", SpanType::Word, 4}
        },
        "CJK word"
    );

    Check(
        "한국어",
        {
            {"한국어", SpanType::Word, 3}
        },
        "Korean word"
    );

    Check(
        "العربية",
        {
            {"العربية", SpanType::Word, 7}
        },
        "Arabic word"
    );

    Check(
        "e\u0301 café",
        {
            {"e\u0301", SpanType::Word, 1},
            {" ", SpanType::Whitespace, 1},
            {"café", SpanType::Word, 4}
        },
        "Combining Latin"
    );

    Check(
        "12345",
        {
            {"12345", SpanType::Number, 5}
        },
        "Number"
    );

    Check(
        "   \t",
        {
            {"   \t", SpanType::Whitespace, 4}
        },
        "Whitespace"
    );

    Check(
        "₹$€",
        {
            {"₹", SpanType::Symbol, 1},
            {"$", SpanType::Symbol, 1},
            {"€", SpanType::Symbol, 1}
        },
        "Currency symbols"
    );

    Check(
        "∑∞→",
        {
            {"∑", SpanType::Symbol, 1},
            {"∞", SpanType::Symbol, 1},
            {"→", SpanType::Symbol, 1}
        },
        "Math symbols"
    );

    Check(
        "👍🏽❤️👨‍👩‍👧‍👦",
        {
            {"👍🏽", SpanType::Emoji, 1},
            {"❤️", SpanType::Emoji, 1},
            {"👨‍👩‍👧‍👦", SpanType::Emoji, 1}
        },
        "Emoji"
    );

    Check(
        "Hello தமிழ் 123 ₹ 👍🏽!",
        {
            {"Hello", SpanType::Word, 5},
            {" ", SpanType::Whitespace, 1},
            {"தமிழ்", SpanType::Word, 3},
            {" ", SpanType::Whitespace, 1},
            {"123", SpanType::Number, 3},
            {" ", SpanType::Whitespace, 1},
            {"₹", SpanType::Symbol, 1},
            {" ", SpanType::Whitespace, 1},
            {"👍🏽", SpanType::Emoji, 1},
            {"!", SpanType::Punctuation, 1}
        },
        "Mixed"
    );

    {
        auto result =
            PreTokenizer::Split("");

        Expect(
            result.Ok(),
            "Empty input accepted"
        );

        Expect(
            result.Ok() &&
            result.Value().empty(),
            "Empty input zero spans"
        );
    }

    {
        std::string invalid("\x80", 1);

        auto result =
            PreTokenizer::Split(invalid);

        Expect(
            result.Failed(),
            "Invalid UTF8 rejected"
        );
    }

    return Summary();
}
