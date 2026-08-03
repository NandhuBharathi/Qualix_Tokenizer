#include "bpe/model_trainer.hpp"

#include <utility>

#include "bpe/corpus_trainer.hpp"
#include "bpe/span_encoder.hpp"
#include "core/error.hpp"
#include "core/status.hpp"

namespace qualix::bpe
{

Result<BpeModel>
BpeModelTrainer::Train(
    std::string_view input,
    const TrainerConfig& config
)
{
    BpeModel model;

    if (input.empty())
        return model;

    /*
     * Pretokenize + symbolize using the model's
     * own vocabulary.
     *
     * This guarantees that the vocabulary used
     * during training is exactly the vocabulary
     * later used during inference.
     */
    auto encoded =
        BpeSpanEncoder::Encode(
            input,
            model.GetVocabulary()
        );

    if (encoded.Failed())
        return encoded.GetStatus();

    SymbolCorpus corpus;

    corpus.reserve(
        encoded.Value().size()
    );

    /*
     * Only non-protected spans participate in
     * BPE merge learning.
     *
     * Protected semantic spans such as URL,
     * date, time, phone, measurement, math, etc.
     * retain their pretokenizer boundary.
     *
     * Every span becomes an independent corpus
     * sequence, therefore BPE can never learn a
     * pair across span boundaries.
     */
    for (auto& span : encoded.Value())
    {
        if (span.protected_span)
            continue;

        if (span.symbols.empty())
            continue;

        corpus.push_back(
            std::move(span.symbols)
        );
    }

    /*
     * BpeCorpusTrainer registers every merged
     * symbol directly inside this vocabulary.
     *
     * Therefore:
     *
     * base symbols + learned merged symbols
     *
     * remain in one stable ID space.
     */
    auto trained =
        BpeCorpusTrainer::Train(
            corpus,
            model.GetVocabulary(),
            config
        );

    model.SetRules(
        std::move(trained.rules)
    );

    /*
     * Training vocabulary construction is now
     * complete.
     *
     * Append the universal 256-byte fallback
     * alphabet only after all learned base and
     * merged symbols have received their stable
     * IDs.
     *
     * From this point onward BpeModel::Encode()
     * can operate without mutating vocabulary.
     */
    if (!model.GetVocabulary().
            EnsureByteFallback())
    {
        return Status{
            ErrorCode::InvalidState
        };
    }

    return model;
}

} // namespace qualix::bpe
