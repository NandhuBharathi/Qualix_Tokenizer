#pragma once

#include <string_view>
#include <vector>

#include "bpe/symbol.hpp"
#include "bpe/vocabulary.hpp"
#include "core/result.hpp"
#include "pretokenizer/span_type.hpp"

namespace qualix::bpe
{

struct EncodedSpan
{
    std::vector<SymbolId> symbols;

    pretokenizer::SpanType type =
        pretokenizer::SpanType::Unknown;

    bool protected_span = false;
};

using EncodedSpans =
    std::vector<EncodedSpan>;

class BpeSpanEncoder
{
public:
    [[nodiscard]]
    static Result<EncodedSpans> Encode(
        std::string_view input,
        Vocabulary& vocabulary
    );
};

} // namespace qualix::bpe
