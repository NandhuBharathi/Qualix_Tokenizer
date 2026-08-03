
#include <iostream>
#include <string>
#include <unordered_set>

#include "bpe/model_trainer.hpp"
#include "bpe/serializer.hpp"

using namespace qualix;
using namespace qualix::bpe;

int main()
{
    const std::string corpus =
        "hello hello world world "
        "தமிழ் தமிழ் வணக்கம் வணக்கம் "
        "namma namma tokenizer tokenizer "
        "abab baba abab baba";

    TrainerConfig config;
    config.max_merges = 64;
    config.min_frequency = 2;

    auto trained =
        BpeModelTrainer::Train(
            corpus,
            config
        );

    if (trained.Failed())
    {
        std::cout << "[FAIL] training\n";
        return 1;
    }

    auto& model = trained.Value();
    const auto& vocabulary =
        model.GetVocabulary();
    const auto& rules =
        model.GetRules();

    /*
     * 1. Vocabulary must be dense:
     *
     * 1 ... VocabularySize()
     *
     * ID 0 remains InvalidSymbolId.
     */
    if (vocabulary.Contains(InvalidSymbolId))
    {
        std::cout << "[FAIL] invalid ID 0 occupied\n";
        return 1;
    }

    for (usize i = 1;
         i <= vocabulary.Size();
         ++i)
    {
        const auto symbol =
            vocabulary.Find(
                static_cast<SymbolId>(i)
            );

        if (!symbol.has_value() ||
            symbol->empty())
        {
            std::cout << "[FAIL] vocabulary ID hole\n";
            return 1;
        }
    }

    /*
     * 2. Byte fallback alphabet must be complete.
     */
    if (!vocabulary.HasByteFallback())
    {
        std::cout << "[FAIL] byte fallback missing\n";
        return 1;
    }

    std::unordered_set<SymbolId> byte_ids;

    for (usize i = 0; i < 256; ++i)
    {
        const auto id =
            vocabulary.FindByte(
                static_cast<u8>(i)
            );

        if (!id.has_value() ||
            *id == InvalidSymbolId)
        {
            std::cout << "[FAIL] fallback byte missing\n";
            return 1;
        }

        byte_ids.insert(*id);
    }

    if (byte_ids.size() != 256)
    {
        std::cout << "[FAIL] fallback IDs not unique\n";
        return 1;
    }

    /*
     * 3. Every rule must reference valid symbols.
     */
    for (const auto& rule : rules)
    {
        if (!rule.Valid() ||
            !vocabulary.Contains(rule.pair.left) ||
            !vocabulary.Contains(rule.pair.right) ||
            !vocabulary.Contains(rule.merged_symbol))
        {
            std::cout << "[FAIL] invalid merge reference\n";
            return 1;
        }

        const auto left =
            vocabulary.Find(rule.pair.left);

        const auto right =
            vocabulary.Find(rule.pair.right);

        const auto merged =
            vocabulary.Find(rule.merged_symbol);

        if (!left.has_value() ||
            !right.has_value() ||
            !merged.has_value())
        {
            std::cout << "[FAIL] merge lookup\n";
            return 1;
        }

        /*
         * 4. merged symbol must equal left + right.
         */
        std::string expected;
        expected.reserve(
            left->size() +
            right->size()
        );

        expected.append(
            left->data(),
            left->size()
        );

        expected.append(
            right->data(),
            right->size()
        );

        if (*merged != expected)
        {
            std::cout << "[FAIL] merge semantics\n";
            return 1;
        }
    }

    /*
     * 5. Training currently creates ranks
     * sequentially from zero.
     */
    for (usize i = 0;
         i < rules.size();
         ++i)
    {
        if (rules[i].rank !=
            static_cast<MergeRank>(i))
        {
            std::cout << "[FAIL] rank ordering\n";
            return 1;
        }
    }

    /*
     * 6. Model must survive serialization and
     * preserve structural counts.
     */
    auto bytes =
        BpeModelSerializer::Serialize(model);

    if (bytes.Failed())
    {
        std::cout << "[FAIL] serialization\n";
        return 1;
    }

    auto restored =
        BpeModelSerializer::Deserialize(
            bytes.Value()
        );

    if (restored.Failed())
    {
        std::cout << "[FAIL] deserialization\n";
        return 1;
    }

    if (restored.Value().VocabularySize() !=
            model.VocabularySize() ||
        restored.Value().RuleCount() !=
            model.RuleCount() ||
        restored.Value().GetRules() !=
            model.GetRules())
    {
        std::cout << "[FAIL] restored structure\n";
        return 1;
    }

    std::cout
        << "[PASS] dense vocabulary IDs\n"
        << "[PASS] byte fallback integrity\n"
        << "[PASS] merge references\n"
        << "[PASS] merge semantics\n"
        << "[PASS] rank ordering\n"
        << "[PASS] serialized structure\n"
        << "Vocabulary : "
        << model.VocabularySize()
        << '\n'
        << "Rules      : "
        << model.RuleCount()
        << '\n';

    return 0;
}
