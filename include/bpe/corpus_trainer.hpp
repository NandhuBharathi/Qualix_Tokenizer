#pragma once

#include <span>
#include <vector>

#include "bpe/merge_rule.hpp"
#include "bpe/symbol.hpp"
#include "bpe/trainer.hpp"
#include "bpe/vocabulary.hpp"

namespace qualix::bpe
{

using SymbolSequence =
    std::vector<SymbolId>;

using SymbolCorpus =
    std::vector<SymbolSequence>;

struct CorpusTrainingResult
{
    SymbolCorpus corpus;
    std::vector<MergeRule> rules;
};

class BpeCorpusTrainer
{
public:
    [[nodiscard]]
    static CorpusTrainingResult Train(
        std::span<const SymbolSequence> corpus,
        Vocabulary& vocabulary,
        const TrainerConfig& config
    );
};

} // namespace qualix::bpe
