#include "bpe/span_encoder.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <utility>

#include "bpe/symbolizer.hpp"
#include "pretokenizer/pretokenizer.hpp"

namespace qualix::bpe
{

Result<EncodedSpans>
BpeSpanEncoder::Encode(
    std::string_view input,
    Vocabulary& vocabulary
)
{
    using Clock =
        std::chrono::steady_clock;

    EncodedSpans output;

    if (input.empty())
        return output;

    const auto total_start =
        Clock::now();

    const auto pretok_start =
        Clock::now();

    const auto split =
        pretokenizer::PreTokenizer::Split(
            input
        );

    const auto pretok_end =
        Clock::now();

    if (split.Failed())
        return split.GetStatus();

    const auto& spans =
        split.Value();

    output.reserve(
        spans.size()
    );

    double symbolizer_seconds = 0.0;
    double construction_seconds = 0.0;

    for (const auto& span : spans)
    {
        const std::string_view view =
            span.View(input);

        if (view.empty())
            continue;

        const auto symbol_start =
            Clock::now();

        auto symbolized =
            Symbolizer::Symbolize(
                view,
                vocabulary
            );

        const auto symbol_end =
            Clock::now();

        symbolizer_seconds +=
            std::chrono::duration<double>(
                symbol_end -
                symbol_start
            ).count();

        if (symbolized.Failed())
            return symbolized.GetStatus();

        const auto construction_start =
            Clock::now();

        output.push_back(
            EncodedSpan{
                std::move(
                    symbolized.Value()
                ),
                span.type,
                span.Protected()
            }
        );

        const auto construction_end =
            Clock::now();

        construction_seconds +=
            std::chrono::duration<double>(
                construction_end -
                construction_start
            ).count();
    }

    const auto total_end =
        Clock::now();

    const double pretok_seconds =
        std::chrono::duration<double>(
            pretok_end -
            pretok_start
        ).count();

    const double total_seconds =
        std::chrono::duration<double>(
            total_end -
            total_start
        ).count();

    double other_seconds =
        total_seconds -
        pretok_seconds -
        symbolizer_seconds -
        construction_seconds;

    if (other_seconds < 0.0)
        other_seconds = 0.0;

    std::cout
        << std::fixed
        << std::setprecision(3)
        << "\n--- #83 SPAN ENCODER PROFILE ---\n"
        << "Pretokenizer      : "
        << pretok_seconds
        << " s\n"
        << "Symbolizer        : "
        << symbolizer_seconds
        << " s\n"
        << "Span construction : "
        << construction_seconds
        << " s\n"
        << "Other             : "
        << other_seconds
        << " s\n"
        << "Total             : "
        << total_seconds
        << " s\n"
        << "Spans             : "
        << spans.size()
        << '\n'
        << "-------------------------------\n";

    return output;
}

} // namespace qualix::bpe
