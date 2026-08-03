#pragma once

#include <span>
#include <vector>

#include "bpe/merge_rule.hpp"
#include "bpe/symbol.hpp"
#include "core/types.hpp"

namespace qualix::bpe
{

struct TrainerConfig
{
    usize max_merges = 0;
    usize min_frequency = 2;
};

struct TrainingResult
{
    std::vector<SymbolId> symbols;
    std::vector<MergeRule> rules;
};

class BpeTrainer
{
public:
    [[nodiscard]]
    static TrainingResult Train(
        std::span<const SymbolId> symbols,
        SymbolId first_available_id,
        const TrainerConfig& config
    );
};

} // namespace qualix::bpe
