#pragma once

#include <span>
#include <string_view>
#include <vector>

#include "bpe/merge_rule.hpp"
#include "bpe/symbol.hpp"
#include "bpe/vocabulary.hpp"
#include "core/result.hpp"
#include "core/types.hpp"

namespace qualix::bpe
{

class BpeModel
{
public:
    BpeModel() = default;

    [[nodiscard]]
    Vocabulary& GetVocabulary() noexcept;

    [[nodiscard]]
    const Vocabulary& GetVocabulary() const noexcept;

    [[nodiscard]]
    std::vector<MergeRule>& GetRules() noexcept;

    [[nodiscard]]
    const std::vector<MergeRule>& GetRules() const noexcept;

    void SetRules(
        std::vector<MergeRule> rules
    );

    [[nodiscard]]
    Result<std::vector<SymbolId>> Encode(
        std::string_view input
    );

    [[nodiscard]]
    std::vector<SymbolId> EncodeSymbols(
        std::span<const SymbolId> symbols
    ) const;

    [[nodiscard]]
    Result<std::string> Decode(
        std::span<const SymbolId> symbols
    ) const;

    [[nodiscard]]
    usize VocabularySize() const noexcept;

    [[nodiscard]]
    usize RuleCount() const noexcept;

    [[nodiscard]]
    bool Empty() const noexcept;

private:
    Vocabulary vocabulary_;
    std::vector<MergeRule> rules_;
};

} // namespace qualix::bpe
