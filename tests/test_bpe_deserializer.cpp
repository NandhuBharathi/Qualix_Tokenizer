#include <iostream>
#include <span>
#include <string>
#include <vector>

#include "bpe/binary_format.hpp"
#include "bpe/deserializer.hpp"
#include "bpe/model_trainer.hpp"
#include "bpe/serializer.hpp"
#include "bpe/trainer.hpp"

using namespace qualix;
using namespace qualix::bpe;

namespace
{

usize tests_run = 0;
usize tests_passed = 0;

void Expect(
    bool condition,
    const char* name
)
{
    ++tests_run;

    if (condition)
    {
        ++tests_passed;

        std::cout
            << "[PASS] "
            << name
            << '\n';
    }
    else
    {
        std::cout
            << "[FAIL] "
            << name
            << '\n';
    }
}

std::span<const u8> Span(
    const std::vector<u8>& data
)
{
    return {
        data.data(),
        data.size()
    };
}

} // namespace

int main()
{
    TrainerConfig config;

    config.max_merges = 32;
    config.min_frequency = 2;

    const std::string corpus =
        "hello hello hello "
        "தமிழ் தமிழ் தமிழ் "
        "token tokenizer token";

    auto trained =
        BpeModelTrainer::Train(
            corpus,
            config
        );

    Expect(
        trained.Ok(),
        "Training succeeds"
    );

    if (trained.Failed())
        return 1;

    auto serialized =
        BpeModelSerializer::Serialize(
            trained.Value()
        );

    Expect(
        serialized.Ok(),
        "Serialization succeeds"
    );

    if (serialized.Failed())
        return 1;

    const auto& bytes =
        serialized.Value();

    Expect(
        !bytes.empty(),
        "Serialized model non-empty"
    );

    /*
     * Round trip.
     */

    auto loaded =
        BpeModelDeserializer::Deserialize(
            Span(bytes)
        );

    Expect(
        loaded.Ok(),
        "Deserialization succeeds"
    );

    if (loaded.Failed())
        return 1;

    Expect(
        loaded.Value().VocabularySize() ==
        trained.Value().VocabularySize(),
        "Vocabulary size preserved"
    );

    Expect(
        loaded.Value().RuleCount() ==
        trained.Value().RuleCount(),
        "Rule count preserved"
    );

    /*
     * Every exact vocabulary ID must survive.
     */

    bool vocabulary_equal = true;

    for (SymbolId id = 1;
         id <
         trained.Value()
             .GetVocabulary()
             .NextId();
         ++id)
    {
        const auto original =
            trained.Value()
                .GetVocabulary()
                .Find(id);

        const auto restored =
            loaded.Value()
                .GetVocabulary()
                .Find(id);

        if (!original.has_value() ||
            !restored.has_value() ||
            *original != *restored)
        {
            vocabulary_equal = false;
            break;
        }
    }

    Expect(
        vocabulary_equal,
        "Exact vocabulary IDs preserved"
    );

    /*
     * Merge rules must survive exactly.
     */

    Expect(
        loaded.Value().GetRules() ==
        trained.Value().GetRules(),
        "Merge rules preserved exactly"
    );

    /*
     * Encoding behavior must remain identical.
     */

    auto original_encoded =
        trained.Value().Encode(
            "hello tokenizer தமிழ்"
        );

    auto restored_encoded =
        loaded.Value().Encode(
            "hello tokenizer தமிழ்"
        );

    Expect(
        original_encoded.Ok() &&
        restored_encoded.Ok(),
        "Both models encode successfully"
    );

    if (original_encoded.Ok() &&
        restored_encoded.Ok())
    {
        Expect(
            original_encoded.Value() ==
            restored_encoded.Value(),
            "Encode result identical after round trip"
        );
    }

    /*
     * Empty input rejected.
     */

    {
        const std::vector<u8> empty;

        auto result =
            BpeModelDeserializer::Deserialize(
                Span(empty)
            );

        Expect(
            result.Failed(),
            "Empty binary rejected"
        );
    }

    /*
     * Truncated binary rejected.
     */

    if (bytes.size() > 1)
    {
        for (usize cut = 0;
             cut < bytes.size();
             ++cut)
        {
            std::vector<u8> truncated(
                bytes.begin(),
                bytes.begin() + cut
            );

            auto result =
                BpeModelDeserializer::Deserialize(
                    Span(truncated)
                );

            if (result.Ok())
            {
                Expect(
                    false,
                    "All truncated binaries rejected"
                );

                goto truncated_done;
            }
        }

        Expect(
            true,
            "All truncated binaries rejected"
        );
    }

truncated_done:

    /*
     * Invalid magic rejected.
     */

    {
        auto corrupted = bytes;

        corrupted[0] ^= 0xFFu;

        auto result =
            BpeModelDeserializer::Deserialize(
                Span(corrupted)
            );

        Expect(
            result.Failed(),
            "Invalid magic rejected"
        );
    }

    /*
     * Unsupported version rejected.
     *
     * Header:
     * magic = first 8 bytes
     * version = next 4 bytes little-endian
     */

    {
        auto corrupted = bytes;

        if (corrupted.size() >= 12)
        {
            const usize offset =
                BpeBinaryMagic.size();

            corrupted[offset + 0] =
                0xFF;

            corrupted[offset + 1] =
                0xFF;

            corrupted[offset + 2] =
                0xFF;

            corrupted[offset + 3] =
                0x7F;

            auto result =
                BpeModelDeserializer::Deserialize(
                    Span(corrupted)
                );

            Expect(
                result.Failed(),
                "Unsupported version rejected"
            );
        }
    }

    /*
     * Unsupported flags rejected.
     */

    {
        auto corrupted = bytes;

        if (corrupted.size() >= 16)
        {
            const usize offset =
                BpeBinaryMagic.size() + 4;

            corrupted[offset] = 1;

            auto result =
                BpeModelDeserializer::Deserialize(
                    Span(corrupted)
                );

            Expect(
                result.Failed(),
                "Unsupported flags rejected"
            );
        }
    }

    /*
     * Trailing garbage rejected.
     */

    {
        auto corrupted = bytes;

        corrupted.push_back(
            0xAA
        );

        auto result =
            BpeModelDeserializer::Deserialize(
                Span(corrupted)
            );

        Expect(
            result.Failed(),
            "Trailing data rejected"
        );
    }

    /*
     * Unicode vocabulary must survive.
     */

    const auto tamil =
        loaded.Value()
            .GetVocabulary()
            .Find("தமிழ்");

    /*
     * Full word may or may not exist depending
     * on learned merges. Instead verify that
     * Tamil encoding still succeeds.
     */

    auto tamil_encoded =
        loaded.Value().Encode(
            "தமிழ்"
        );

    Expect(
        tamil_encoded.Ok() &&
        !tamil_encoded.Value().empty(),
        "Unicode model survives deserialization"
    );

    (void)tamil;

    std::cout
        << '\n'
        << tests_passed
        << "/"
        << tests_run
        << " tests passed\n";

    return
        tests_passed == tests_run
            ? 0
            : 1;
}
