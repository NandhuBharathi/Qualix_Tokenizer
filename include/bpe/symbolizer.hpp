#pragma once

#include <string_view>
#include <vector>

#include "bpe/symbol.hpp"
#include "bpe/vocabulary.hpp"
#include "core/result.hpp"

namespace qualix::bpe
{

class Symbolizer
{
public:
    /*
     * Training symbolization.
     *
     * Graphemes not already present in the
     * vocabulary are appended to it.
     */
    [[nodiscard]]
    static Result<std::vector<SymbolId>> Symbolize(
        std::string_view input,
        Vocabulary& vocabulary
    );

    /*
     * Inference symbolization.
     *
     * Existing grapheme:
     *     use its normal vocabulary ID.
     *
     * Unknown grapheme:
     *     encode its exact UTF-8 bytes through
     *     the registered byte fallback alphabet.
     *
     * This operation never grows the vocabulary.
     */
    [[nodiscard]]
    static Result<std::vector<SymbolId>>
    SymbolizeWithFallback(
        std::string_view input,
        const Vocabulary& vocabulary
    );
};

} // namespace qualix::bpe
