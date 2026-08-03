#include "bpe/pair_counter.hpp"

namespace qualix::bpe
{

PairFrequencies PairCounter::Count(
    std::span<const SymbolId> symbols
)
{
    PairFrequencies frequencies;

    if (symbols.size() < 2)
        return frequencies;

    frequencies.reserve(
        symbols.size() - 1
    );

    for (usize i = 0;
         i + 1 < symbols.size();
         ++i)
    {
        const Pair pair{
            symbols[i],
            symbols[i + 1]
        };

        ++frequencies[pair];
    }

    return frequencies;
}

std::optional<PairFrequency>
PairCounter::MostFrequent(
    const PairFrequencies& frequencies
) noexcept
{
    if (frequencies.empty())
        return std::nullopt;

    PairFrequency best{};
    bool found = false;

    for (const auto& [pair, frequency] :
         frequencies)
    {
        if (!found ||
            frequency > best.frequency ||
            (
                frequency == best.frequency &&
                (
                    pair.left < best.pair.left ||
                    (
                        pair.left ==
                            best.pair.left &&
                        pair.right <
                            best.pair.right
                    )
                )
            ))
        {
            best = PairFrequency{
                pair,
                frequency
            };

            found = true;
        }
    }

    return best;
}

} // namespace qualix::bpe
