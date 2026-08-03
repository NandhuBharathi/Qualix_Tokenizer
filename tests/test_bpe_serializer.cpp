#include <iostream>
#include <span>
#include <string>
#include <vector>

#include "bpe/binary_format.hpp"
#include "bpe/model_trainer.hpp"
#include "bpe/serializer.hpp"
#include "bpe/trainer.hpp"
#include "core/types.hpp"

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

bool SameVocabulary(
    const BpeModel& a,
    const BpeModel& b
)
{
    if (a.VocabularySize() !=
        b.VocabularySize())
    {
        return false;
    }

    for (usize i = 1;
         i <= a.VocabularySize();
         ++i)
    {
        const auto left =
            a.GetVocabulary().Find(
                static_cast<SymbolId>(i)
            );

        const auto right =
            b.GetVocabulary().Find(
                static_cast<SymbolId>(i)
            );

        if (!left.has_value() ||
            !right.has_value() ||
            *left != *right)
        {
            return false;
        }
    }

    return true;
}

bool SameRules(
    const BpeModel& a,
    const BpeModel& b
)
{
    return
        a.GetRules() ==
        b.GetRules();
}

void WriteU32At(
    std::vector<u8>& data,
    usize offset,
    u32 value
)
{
    if (offset + 4 >
        data.size())
    {
        return;
    }

    for (usize i = 0;
         i < 4;
         ++i)
    {
        data[offset + i] =
            static_cast<u8>(
                (value >> (i * 8)) &
                0xFFu
            );
    }
}

} // namespace

int main()
{
    const TrainerConfig config{
        32,
        2
    };

    const std::string corpus =
        "hello hello hello "
        "தமிழ் தமிழ் தமிழ் "
        "token token token";

    auto trained =
        BpeModelTrainer::Train(
            corpus,
            config
        );

    Expect(
        trained.Ok(),
        "Model training succeeds"
    );

    if (trained.Failed())
        return 1;

    auto serialized =
        BpeModelSerializer::Serialize(
            trained.Value()
        );

    Expect(
        serialized.Ok(),
        "Model serialization succeeds"
    );

    if (serialized.Failed())
        return 1;

    Expect(
        !serialized.Value().empty(),
        "Serialized model non-empty"
    );

    auto restored =
        BpeModelSerializer::Deserialize(
            serialized.Value()
        );

    Expect(
        restored.Ok(),
        "Model deserialization succeeds"
    );

    if (restored.Failed())
        return 1;

    Expect(
        SameVocabulary(
            trained.Value(),
            restored.Value()
        ),
        "Vocabulary exact-ID round trip"
    );

    Expect(
        SameRules(
            trained.Value(),
            restored.Value()
        ),
        "Merge rules exact round trip"
    );

    Expect(
        trained.Value().VocabularySize() ==
        restored.Value().VocabularySize(),
        "Vocabulary size preserved"
    );

    Expect(
        trained.Value().RuleCount() ==
        restored.Value().RuleCount(),
        "Rule count preserved"
    );

    /*
     * Inference equivalence.
     */

    auto original_encoding =
        trained.Value().Encode(
            "hello"
        );

    auto restored_encoding =
        restored.Value().Encode(
            "hello"
        );

    Expect(
        original_encoding.Ok() &&
        restored_encoding.Ok() &&
        original_encoding.Value() ==
        restored_encoding.Value(),
        "Encoding preserved after round trip"
    );

    /*
     * Empty model must also round-trip.
     */

    {
        BpeModel empty;

        auto bytes =
            BpeModelSerializer::Serialize(
                empty
            );

        Expect(
            bytes.Ok(),
            "Empty model serialization"
        );

        if (bytes.Ok())
        {
            auto decoded =
                BpeModelSerializer::Deserialize(
                    bytes.Value()
                );

            Expect(
                decoded.Ok() &&
                decoded.Value().Empty(),
                "Empty model round trip"
            );
        }
    }

    /*
     * Truncation must be rejected at every
     * possible byte boundary.
     */

    {
        bool rejected_all = true;

        const auto& bytes =
            serialized.Value();

        for (usize size = 0;
             size < bytes.size();
             ++size)
        {
            auto decoded =
                BpeModelSerializer::Deserialize(
                    std::span<const u8>{
                        bytes.data(),
                        size
                    }
                );

            if (decoded.Ok())
            {
                rejected_all = false;
                break;
            }
        }

        Expect(
            rejected_all,
            "All truncated payloads rejected"
        );
    }

    /*
     * Magic corruption.
     */

    {
        auto corrupted =
            serialized.Value();

        corrupted[0] ^= 0xFFu;

        auto decoded =
            BpeModelSerializer::Deserialize(
                corrupted
            );

        Expect(
            decoded.Failed(),
            "Invalid magic rejected"
        );
    }

    /*
     * Version corruption.
     *
     * Header:
     * magic   = 8 bytes
     * version = offset 8
     */

    {
        auto corrupted =
            serialized.Value();

        WriteU32At(
            corrupted,
            8,
            BpeBinaryVersion + 1
        );

        auto decoded =
            BpeModelSerializer::Deserialize(
                corrupted
            );

        Expect(
            decoded.Failed(),
            "Unsupported version rejected"
        );
    }

    /*
     * Flags corruption.
     *
     * flags = offset 12
     */

    {
        auto corrupted =
            serialized.Value();

        WriteU32At(
            corrupted,
            12,
            1
        );

        auto decoded =
            BpeModelSerializer::Deserialize(
                corrupted
            );

        Expect(
            decoded.Failed(),
            "Unsupported flags rejected"
        );
    }

    /*
     * Impossible vocabulary count.
     *
     * vocabulary count = offset 16
     */

    {
        auto corrupted =
            serialized.Value();

        WriteU32At(
            corrupted,
            16,
            0xFFFFFFFFu
        );

        auto decoded =
            BpeModelSerializer::Deserialize(
                corrupted
            );

        Expect(
            decoded.Failed(),
            "Impossible vocabulary count rejected"
        );
    }

    /*
     * Dense ID corruption.
     *
     * First vocabulary ID begins at offset 20
     * whenever vocabulary is non-empty.
     */

    if (trained.Value().VocabularySize() > 0)
    {
        auto corrupted =
            serialized.Value();

        WriteU32At(
            corrupted,
            20,
            2
        );

        auto decoded =
            BpeModelSerializer::Deserialize(
                corrupted
            );

        Expect(
            decoded.Failed(),
            "Non-dense vocabulary ID rejected"
        );
    }

    /*
     * Trailing garbage.
     */

    {
        auto corrupted =
            serialized.Value();

        corrupted.push_back(
            0xAAu
        );

        auto decoded =
            BpeModelSerializer::Deserialize(
                corrupted
            );

        Expect(
            decoded.Failed(),
            "Trailing bytes rejected"
        );
    }

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
