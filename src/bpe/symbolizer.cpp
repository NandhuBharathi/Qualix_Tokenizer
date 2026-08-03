#include "bpe/symbolizer.hpp"

#include <string>
#include <vector>

#include "core/error.hpp"
#include "core/status.hpp"
#include "unicode/grapheme_segmenter.hpp"

namespace qualix::bpe
{

Result<std::vector<SymbolId>>
Symbolizer::Symbolize(
    std::string_view input,
    Vocabulary& vocabulary
)
{
    std::vector<SymbolId> symbols;

    if (input.empty())
        return symbols;

    const auto segmented =
        unicode::GraphemeSegmenter::Segment(
            input
        );

    if (segmented.Failed())
        return segmented.GetStatus();

    const auto& graphemes =
        segmented.Value();

    symbols.reserve(
        graphemes.size()
    );

    for (const auto& grapheme :
         graphemes)
    {
        const std::string_view view =
            grapheme.View(input);

        if (view.empty())
            continue;

        const SymbolId id =
            vocabulary.Add(
                std::string(view)
            );

        if (id == InvalidSymbolId)
            continue;

        symbols.push_back(id);
    }

    return symbols;
}

Result<std::vector<SymbolId>>
Symbolizer::SymbolizeWithFallback(
    std::string_view input,
    const Vocabulary& vocabulary
)
{
    std::vector<SymbolId> symbols;

    if (input.empty())
        return symbols;

    /*
     * Inference fallback requires the complete
     * byte alphabet to already exist.
     *
     * We intentionally do not mutate the
     * vocabulary here.
     */
    if (!vocabulary.HasByteFallback())
    {
        return Status{
            ErrorCode::InvalidState
        };
    }

    const auto segmented =
        unicode::GraphemeSegmenter::Segment(
            input
        );

    if (segmented.Failed())
        return segmented.GetStatus();

    const auto& graphemes =
        segmented.Value();

    /*
     * At minimum one ID per grapheme.
     * Unknown graphemes may expand to several
     * UTF-8 byte IDs.
     */
    symbols.reserve(
        graphemes.size()
    );

    for (const auto& grapheme :
         graphemes)
    {
        const std::string_view view =
            grapheme.View(input);

        if (view.empty())
            continue;

        /*
         * Prefer an exact learned symbol.
         */
        const auto known =
            vocabulary.Find(view);

        if (known.has_value())
        {
            symbols.push_back(
                *known
            );

            continue;
        }

        /*
         * OOV grapheme:
         *
         * preserve every UTF-8 byte exactly.
         */
        for (const unsigned char raw : view)
        {
            const auto byte_id =
                vocabulary.FindByte(
                    static_cast<u8>(
                        raw
                    )
                );

            if (!byte_id.has_value())
            {
                return Status{
                    ErrorCode::InvalidState
                };
            }

            symbols.push_back(
                *byte_id
            );
        }
    }

    return symbols;
}

} // namespace qualix::bpe
