#include "bpe/corpus_trainer.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>

#include "bpe/merger.hpp"
#include "bpe/pair_counter.hpp"

namespace qualix::bpe
{

namespace
{

PairFrequencies CountCorpusPairs(
    const SymbolCorpus& corpus
)
{
    PairFrequencies total;

    for (const auto& sequence : corpus)
    {
        /*
         * Count each sequence independently.
         *
         * This is critical:
         *
         * No pair is ever created between the
         * final symbol of one span and the first
         * symbol of another span.
         */
        const auto frequencies =
            PairCounter::Count(sequence);

        for (const auto& [pair, frequency] :
             frequencies)
        {
            total[pair] += frequency;
        }
    }

    return total;
}

} // namespace

CorpusTrainingResult BpeCorpusTrainer::Train(
    std::span<const SymbolSequence> corpus,
    Vocabulary& vocabulary,
    const TrainerConfig& config
)
{
    CorpusTrainingResult result;

    result.corpus.assign(
        corpus.begin(),
        corpus.end()
    );

    if (result.corpus.empty() ||
        config.max_merges == 0)
    {
        return result;
    }

    result.rules.reserve(
        config.max_merges
    );

    using Clock =
        std::chrono::steady_clock;

    const auto training_start =
        Clock::now();

    double pair_count_seconds = 0.0;
    double best_pair_seconds = 0.0;
    double add_merged_seconds = 0.0;
    double corpus_merge_seconds = 0.0;

    std::cout
        << "\nQUALIX BPE TRAINING\n";

    for (usize round = 0;
         round < config.max_merges;
         ++round)
    {
        const auto pair_start =
            Clock::now();

        const auto frequencies =
            CountCorpusPairs(
                result.corpus
            );

        pair_count_seconds +=
            std::chrono::duration<double>(
                Clock::now() -
                pair_start
            ).count();

        const auto best_start =
            Clock::now();

        const auto best =
            PairCounter::MostFrequent(
                frequencies
            );

        best_pair_seconds +=
            std::chrono::duration<double>(
                Clock::now() -
                best_start
            ).count();

        if (!best.has_value())
            break;

        if (best->frequency <
            config.min_frequency)
        {
            break;
        }

        /*
         * The merged symbol is registered in the
         * vocabulary itself.
         *
         * Example:
         *
         * "t" + "h" -> "th"
         */
        const auto add_start =
            Clock::now();

        const SymbolId merged_symbol =
            vocabulary.AddMerged(
                best->pair.left,
                best->pair.right
            );

        add_merged_seconds +=
            std::chrono::duration<double>(
                Clock::now() -
                add_start
            ).count();

        if (merged_symbol ==
            InvalidSymbolId)
        {
            break;
        }

        const MergeRule rule{
            best->pair,
            merged_symbol,
            static_cast<MergeRank>(round)
        };

        /*
         * Apply the rule independently to every
         * sequence. Boundaries are preserved.
         */
        const auto merge_start =
            Clock::now();

        for (auto& sequence :
             result.corpus)
        {
            sequence =
                Merger::Merge(
                    sequence,
                    rule.pair,
                    rule.merged_symbol
                );
        }

        corpus_merge_seconds +=
            std::chrono::duration<double>(
                Clock::now() -
                merge_start
            ).count();

        result.rules.push_back(
            rule
        );

        const auto now =
            Clock::now();

        const double elapsed =
            std::chrono::duration<double>(
                now - training_start
            ).count();

        const usize completed =
            round + 1;

        const double average =
            elapsed /
            static_cast<double>(
                completed
            );

        const usize remaining =
            config.max_merges >
                    completed
                ? config.max_merges -
                    completed
                : 0;

        const double eta =
            average *
            static_cast<double>(
                remaining
            );

        constexpr usize BarWidth = 30;

        const double progress =
            config.max_merges == 0
                ? 1.0
                : static_cast<double>(
                    completed
                  ) /
                  static_cast<double>(
                    config.max_merges
                  );

        const usize filled =
            static_cast<usize>(
                progress *
                static_cast<double>(
                    BarWidth
                )
            );

        std::cout
            << '\r'
            << '[';

        for (usize i = 0;
             i < BarWidth;
             ++i)
        {
            std::cout
                << (i < filled
                        ? '#'
                        : '-');
        }

        std::cout
            << "] "
            << std::setw(3)
            << static_cast<int>(
                progress * 100.0
            )
            << "% | "
            << completed
            << '/'
            << config.max_merges
            << " | elapsed "
            << std::fixed
            << std::setprecision(1)
            << elapsed
            << "s | ETA ~"
            << eta
            << "s"
            << std::flush;
    }

    const double total_time =
        std::chrono::duration<double>(
            Clock::now() -
            training_start
        ).count();

    std::cout
        << "\nLearned    : "
        << result.rules.size()
        << " merges"
        << "\nTotal time : "
        << std::fixed
        << std::setprecision(3)
        << total_time
        << " s\n";

    std::cout
        << "\n--- #81 TRAINING PROFILE ---\n"
        << "Pair counting : "
        << pair_count_seconds
        << " s\n"
        << "Best pair     : "
        << best_pair_seconds
        << " s\n"
        << "AddMerged     : "
        << add_merged_seconds
        << " s\n"
        << "Corpus merge  : "
        << corpus_merge_seconds
        << " s\n"
        << "Other         : "
        << (
            total_time -
            pair_count_seconds -
            best_pair_seconds -
            add_merged_seconds -
            corpus_merge_seconds
        )
        << " s\n"
        << "----------------------------\n";

    return result;
}

} // namespace qualix::bpe
