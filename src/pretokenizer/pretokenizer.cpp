#include "pretokenizer/pretokenizer.hpp"

#include <memory>

#include "pretokenizer/grapheme_classifier.hpp"
#include "rules/rule_engine.hpp"
#include "rules/url_rule.hpp"
#include "rules/email_rule.hpp"
#include "rules/currency_rule.hpp"
#include "rules/percentage_rule.hpp"
#include "rules/number_rule.hpp"
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

SpanType ToSpanType(
    rules::RuleType type
) noexcept
{
    switch (type)
    {
        case rules::RuleType::Url:
            return SpanType::Url;

        case rules::RuleType::Email:
            return SpanType::Email;

        case rules::RuleType::Date:
            return SpanType::Date;

        case rules::RuleType::Time:
            return SpanType::Time;

        case rules::RuleType::Number:
            return SpanType::Number;

        case rules::RuleType::Currency:
            return SpanType::Currency;

        case rules::RuleType::Percentage:
            return SpanType::Percentage;

        case rules::RuleType::Math:
            return SpanType::Math;

        case rules::RuleType::Code:
            return SpanType::Code;

        case rules::RuleType::None:
            return SpanType::Unknown;
    }

    return SpanType::Unknown;
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

rules::RuleEngine CreateRuleEngine()
{
    rules::RuleEngine engine;

    engine.Add(
        std::make_unique<rules::EmailRule>()
    );

    engine.Add(
        std::make_unique<rules::PercentageRule>()
    );

    engine.Add(
        std::make_unique<rules::UrlRule>()
    );

    engine.Add(
        std::make_unique<rules::CurrencyRule>()
    );

    engine.Add(
        std::make_unique<rules::NumberRule>()
    );

    return engine;
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

    auto rule_engine =
        CreateRuleEngine();

    usize index = 0;

    while (index < graphemes.size())
    {
        const auto& grapheme =
            graphemes[index];

        const auto rule_match =
            rule_engine.MatchAt(
                input,
                grapheme.byte_start
            );

        if (rule_match.Matched())
        {
            const usize match_end =
                rule_match.ByteEnd();

            usize end_index = index;

            while (end_index < graphemes.size() &&
                   graphemes[end_index].byte_start <
                       match_end)
            {
                ++end_index;
            }

            if (end_index > index &&
                graphemes[end_index - 1].ByteEnd() ==
                    match_end)
            {
                output.push_back(
                    Span{
                        rule_match.byte_start,
                        rule_match.byte_length,
                        index,
                        end_index - index,
                        ToSpanType(
                            rule_match.type
                        ),
                        SpanPolicy::Protected
                    }
                );

                index = end_index;
                continue;
            }
        }

        const auto view =
            grapheme.View(input);

        const auto grapheme_class =
            GraphemeClassifier::Classify(view);

        const auto span_type =
            ToSpanType(grapheme_class);

        if (!output.empty() &&
            !output.back().Protected() &&
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

            ++index;
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

        ++index;
    }

    return output;
}

} // namespace qualix::pretokenizer
