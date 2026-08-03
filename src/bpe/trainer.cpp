#include "bpe/trainer.hpp"

#include <vector>

#include "bpe/merger.hpp"
#include "bpe/pair_counter.hpp"
#include "bpe/symbol_allocator.hpp"

namespace qualix::bpe
{

TrainingResult BpeTrainer::Train(
    std::span<const SymbolId> symbols,
    SymbolId first_available_id,
    const TrainerConfig& config
)
{
    TrainingResult result;

    result.symbols.assign(
        symbols.begin(),
        symbols.end()
    );

    if (result.symbols.size() < 2 ||
        config.max_merges == 0)
    {
        return result;
    }

    SymbolAllocator allocator{
        first_available_id
    };

    result.rules.reserve(
        config.max_merges
    );

    for (usize round = 0;
         round < config.max_merges;
         ++round)
    {
        const auto frequencies =
            PairCounter::Count(
                result.symbols
            );

        const auto best =
            PairCounter::MostFrequent(
                frequencies
            );

        if (!best.has_value())
            break;

        if (best->frequency <
            config.min_frequency)
        {
            break;
        }

        const auto allocated =
            allocator.Allocate();

        if (!allocated.has_value())
            break;

        const SymbolId merged_id =
            *allocated;

        const MergeRule rule{
            best->pair,
            merged_id,
            round
        };

        result.symbols =
            Merger::Merge(
                result.symbols,
                rule.pair,
                rule.merged_symbol
            );

        result.rules.push_back(
            rule
        );
    }

    return result;
}

} // namespace qualix::bpe
