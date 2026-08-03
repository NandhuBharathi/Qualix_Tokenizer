#include "pretokenizer/pretokenizer.hpp"

#include <memory>
#include <chrono>
#include <iostream>
#include <iomanip>

#include "pretokenizer/grapheme_classifier.hpp"
#include "rules/rule_engine.hpp"
#include "rules/url_rule.hpp"
#include "rules/email_rule.hpp"
#include "rules/currency_rule.hpp"
#include "rules/date_rule.hpp"
#include "rules/time_rule.hpp"
#include "rules/phone_rule.hpp"
#include "rules/measurement_rule.hpp"
#include "rules/math_rule.hpp"
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

        case rules::RuleType::Phone:
            return SpanType::Phone;

        case rules::RuleType::Number:
            return SpanType::Number;

        case rules::RuleType::Currency:
            return SpanType::Currency;

        case rules::RuleType::Percentage:
            return SpanType::Percentage;

        case rules::RuleType::Measurement:
            return SpanType::Measurement;

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
        std::make_unique<rules::DateRule>()
    );

    engine.Add(
        std::make_unique<rules::TimeRule>()
    );

    engine.Add(
        std::make_unique<rules::CurrencyRule>()
    );

    engine.Add(
        std::make_unique<rules::MeasurementRule>()
    );

    engine.Add(
        std::make_unique<rules::PhoneRule>()
    );

    engine.Add(
        std::make_unique<rules::MathRule>()
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
    using ProfileClock =
        std::chrono::steady_clock;

    double profile_segment = 0.0;
    double profile_rules = 0.0;
    double profile_advance = 0.0;
    double profile_classifier = 0.0;
    double profile_span = 0.0;

    const auto profile_total_start =
        ProfileClock::now();

    const auto profile_segment_start =
        ProfileClock::now();

    auto segmented =
        unicode::GraphemeSegmenter::Segment(input);

    const auto profile_segment_end =
        ProfileClock::now();

    profile_segment =
        std::chrono::duration<double>(
            profile_segment_end -
            profile_segment_start
        ).count();

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

        const auto profile_rule_start =
            ProfileClock::now();

        const auto rule_match =
            rule_engine.MatchAt(
                input,
                grapheme.byte_start
            );

        const auto profile_rule_end =
            ProfileClock::now();

        profile_rules +=
            std::chrono::duration<double>(
                profile_rule_end -
                profile_rule_start
            ).count();

        if (rule_match.Matched())
        {
            const usize match_end =
                rule_match.ByteEnd();

            usize end_index = index;

            const auto profile_advance_start =
                ProfileClock::now();

            while (end_index < graphemes.size() &&
                   graphemes[end_index].byte_start <
                       match_end)
            {
                ++end_index;
            }

            const auto profile_advance_end =
                ProfileClock::now();

            profile_advance +=
                std::chrono::duration<double>(
                    profile_advance_end -
                    profile_advance_start
                ).count();

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

        const auto profile_classifier_start =
            ProfileClock::now();

        const auto grapheme_class =
            GraphemeClassifier::Classify(view);

        const auto profile_classifier_end =
            ProfileClock::now();

        profile_classifier +=
            std::chrono::duration<double>(
                profile_classifier_end -
                profile_classifier_start
            ).count();

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

        const auto profile_span_start =
            ProfileClock::now();

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

        const auto profile_span_end =
            ProfileClock::now();

        profile_span +=
            std::chrono::duration<double>(
                profile_span_end -
                profile_span_start
            ).count();

        ++index;
    }

    const auto profile_total_end =
        ProfileClock::now();

    const double profile_total =
        std::chrono::duration<double>(
            profile_total_end -
            profile_total_start
        ).count();

    const double profile_known =
        profile_segment +
        profile_rules +
        profile_advance +
        profile_classifier +
        profile_span;

    std::cout
        << "\n--- #87 PRETOKENIZER INTERNAL PROFILE ---\n"
        << "Grapheme segment : "
        << std::fixed << std::setprecision(3)
        << profile_segment << " s\n"
        << "Rule matching     : "
        << profile_rules << " s\n"
        << "Matched advance   : "
        << profile_advance << " s\n"
        << "Classifier        : "
        << profile_classifier << " s\n"
        << "Span construction : "
        << profile_span << " s\n"
        << "Unaccounted       : "
        << (profile_total - profile_known)
        << " s\n"
        << "Total             : "
        << profile_total << " s\n"
        << "-----------------------------------------\n";

    return output;
}

} // namespace qualix::pretokenizer
