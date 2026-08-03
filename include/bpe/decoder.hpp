#pragma once

#include <span>
#include <string>

#include "bpe/model.hpp"
#include "bpe/symbol.hpp"
#include "bpe/vocabulary.hpp"
#include "core/result.hpp"

namespace qualix::bpe
{

class BpeDecoder
{
public:
    [[nodiscard]]
    static Result<std::string> Decode(
        std::span<const SymbolId> symbols,
        const Vocabulary& vocabulary
    );

    [[nodiscard]]
    static Result<std::string> Decode(
        std::span<const SymbolId> symbols,
        const BpeModel& model
    );
};

} // namespace qualix::bpe
