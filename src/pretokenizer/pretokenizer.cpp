#include "pretokenizer/pretokenizer.hpp"

#include "pretokenizer/grapheme_classifier.hpp"
#include "unicode/grapheme_segmenter.hpp"

namespace qualix::pretokenizer
{

namespace
{

SpanType ToSpanType(
    GraphemeClass type
) noexcept
{
    switch (type)
    {
        case GraphemeClass::Letter:
            return SpanType::Word;

        case GraphemeClass::Number:
            return SpanType::Number;

        case GraphemeClass::Whitespace:
            return SpanType::Whitespace;

        case GraphemeClass::Punctuation:
            return SpanType::Punctuation;

        case GraphemeClass::Symbol:
            return SpanType::Symbol;

        case GraphemeClass::Emoji:
            return SpanType::Emoji;

        case GraphemeClass::Control:
        case GraphemeClass::Other:
            return SpanType::Other;
    }

    return SpanType::Other;
}

bool CanMerge(
    SpanType type
) noexcept
{
    return
        type == SpanType::Word ||
        type == SpanType::Number ||
        type == SpanType::Whitespace;
}

} // namespace

Result<std::vector<Span>> PreTokenizer::Split(
    std::string_view input
)
{
    auto segmented =
        unicode::GraphemeSegmenter::Segment(input);

    if (segmented.Failed())
        return Status::Failure(
            ErrorCode::InvalidUtf8
        );

    const auto& graphemes =
        segmented.Value();

    std::vector<Span> output;

    if (graphemes.empty())
        return output;

    output.reserve(graphemes.size());

    for (usize index = 0;
         index < graphemes.size();
         ++index)
    {
        const auto& grapheme =
            graphemes[index];

        const auto view =
            grapheme.View(input);

        const auto grapheme_class =
            GraphemeClassifier::Classify(view);

        const auto span_type =
            ToSpanType(grapheme_class);

        if (!output.empty() &&
            CanMerge(span_type) &&
            output.back().type == span_type &&
            output.back().ByteEnd() ==
                grapheme.byte_start &&
            output.back().GraphemeEnd() ==
                index)
        {
            auto& current =
                output.back();

            current.byte_length +=
                grapheme.byte_length;

            current.grapheme_count += 1;

            continue;
        }

        output.push_back(
            Span{
                grapheme.byte_start,
                grapheme.byte_length,
                index,
                1,
                span_type,
                SpanPolicy::Splittable
            }
        );
    }

    return output;
}

} // namespace qualix::pretokenizer
