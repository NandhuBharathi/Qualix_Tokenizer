#include "bpe/model.hpp"

#include <utility>

#include "bpe/decoder.hpp"
#include "bpe/encoder.hpp"
#include "bpe/symbolizer.hpp"

namespace qualix::bpe
{

Vocabulary&
BpeModel::GetVocabulary() noexcept
{
    return vocabulary_;
}

const Vocabulary&
BpeModel::GetVocabulary() const noexcept
{
    return vocabulary_;
}

std::vector<MergeRule>&
BpeModel::GetRules() noexcept
{
    return rules_;
}

const std::vector<MergeRule>&
BpeModel::GetRules() const noexcept
{
    return rules_;
}

void BpeModel::SetRules(
    std::vector<MergeRule> rules
)
{
    rules_ = std::move(rules);
}

Result<std::vector<SymbolId>>
BpeModel::Encode(
    std::string_view input
)
{
    /*
     * Model inference is vocabulary-frozen.
     *
     * Learned graphemes retain their existing
     * IDs. Any unseen grapheme is represented
     * through the pre-registered byte fallback
     * alphabet.
     *
     * Encoding must never grow a trained model's
     * vocabulary.
     */
    auto symbolized =
        Symbolizer::SymbolizeWithFallback(
            input,
            vocabulary_
        );

    if (symbolized.Failed())
        return symbolized.GetStatus();

    return BpeEncoder::Encode(
        symbolized.Value(),
        rules_
    );
}

std::vector<SymbolId>
BpeModel::EncodeSymbols(
    std::span<const SymbolId> symbols
) const
{
    return BpeEncoder::Encode(
        symbols,
        rules_
    );
}

Result<std::string>
BpeModel::Decode(
    std::span<const SymbolId> symbols
) const
{
    return BpeDecoder::Decode(
        symbols,
        vocabulary_
    );
}

usize
BpeModel::VocabularySize() const noexcept
{
    return vocabulary_.Size();
}

usize
BpeModel::RuleCount() const noexcept
{
    return rules_.size();
}

bool
BpeModel::Empty() const noexcept
{
    return
        vocabulary_.Size() == 0 &&
        rules_.empty();
}

} // namespace qualix::bpe
