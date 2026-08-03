#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "bpe/model.hpp"
#include "bpe/model_io.hpp"
#include "bpe/model_trainer.hpp"
#include "bpe/symbol.hpp"
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
    const BpeModel& left,
    const BpeModel& right
)
{
    if (left.VocabularySize() !=
        right.VocabularySize())
    {
        return false;
    }

    for (usize i = 1;
         i <= left.VocabularySize();
         ++i)
    {
        const SymbolId id =
            static_cast<SymbolId>(i);

        const auto a =
            left.GetVocabulary().Find(id);

        const auto b =
            right.GetVocabulary().Find(id);

        if (!a.has_value() ||
            !b.has_value() ||
            *a != *b)
        {
            return false;
        }
    }

    return true;
}

bool SameEncoding(
    BpeModel& left,
    BpeModel& right,
    std::string_view text
)
{
    auto a =
        left.Encode(text);

    auto b =
        right.Encode(text);

    return
        a.Ok() &&
        b.Ok() &&
        a.Value() == b.Value();
}

void PrintEncoding(
    BpeModel& model,
    std::string_view text
)
{
    auto encoded =
        model.Encode(text);

    std::cout
        << "[ENCODE] "
        << text
        << " -> ";

    if (encoded.Failed())
    {
        std::cout
            << "<FAILED>\n";

        return;
    }

    for (const SymbolId id :
         encoded.Value())
    {
        std::cout
            << id
            << ' ';
    }

    std::cout << '\n';
}

} // namespace

int main()
{
    /*
     * Repeated corpus gives BPE enough frequency
     * to learn meaningful merge rules.
     *
     * English + Tamil + mixed text are included
     * so persistence is tested against the
     * Unicode-first Qualix pipeline.
     */
    const std::string corpus =
        "hello hello hello hello "
        "token token token token "
        "tokenizer tokenizer tokenizer "
        "தமிழ் தமிழ் தமிழ் தமிழ் "
        "வணக்கம் வணக்கம் வணக்கம் "
        "hello தமிழ் hello தமிழ் "
        "qualix qualix qualix qualix";

    const TrainerConfig config{
        64,
        2
    };

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

    BpeModel original =
        std::move(
            trained.Value()
        );

    Expect(
        original.VocabularySize() > 0,
        "Trained vocabulary non-empty"
    );

    Expect(
        original.RuleCount() > 0,
        "BPE merge rules learned"
    );

    const usize original_vocab_size =
        original.VocabularySize();

    const usize original_rule_count =
        original.RuleCount();

    std::cout
        << "\nVocabulary size : "
        << original_vocab_size
        << '\n'
        << "Rule count      : "
        << original_rule_count
        << "\n\n";

    /*
     * Capture inference results before saving.
     */

    const std::vector<std::string>
        inference_inputs{
            "hello",
            "token",
            "tokenizer",
            "தமிழ்",
            "வணக்கம்",
            "qualix",
            "hello தமிழ்",
            "qualix tokenizer"
        };

    std::vector<
        std::vector<SymbolId>
    > expected_encodings;

    expected_encodings.reserve(
        inference_inputs.size()
    );

    bool original_encoding_ok = true;

    for (const auto& input :
         inference_inputs)
    {
        auto encoded =
            original.Encode(input);

        if (encoded.Failed())
        {
            original_encoding_ok =
                false;

            break;
        }

        expected_encodings.push_back(
            encoded.Value()
        );
    }

    Expect(
        original_encoding_ok,
        "Original inference succeeds"
    );

    /*
     * Persist to disk.
     */

    const std::filesystem::path path =
        std::filesystem::temp_directory_path() /
        "qualix_bpe_end_to_end.qlxbpe";

    std::error_code ec;

    std::filesystem::remove(
        path,
        ec
    );

    const auto saved =
        BpeModelIO::Save(
            original,
            path
        );

    Expect(
        saved.Ok(),
        "Model saved to disk"
    );

    if (saved.Failed())
        return 1;

    Expect(
        std::filesystem::exists(path),
        "Model file exists"
    );

    Expect(
        std::filesystem::file_size(path) > 0,
        "Model file non-empty"
    );

    /*
     * Load a completely fresh model from disk.
     *
     * No vocabulary/rule state is reused from
     * the original object.
     */

    auto loaded_result =
        BpeModelIO::Load(
            path
        );

    Expect(
        loaded_result.Ok(),
        "Fresh model loaded from disk"
    );

    if (loaded_result.Failed())
    {
        std::filesystem::remove(
            path,
            ec
        );

        return 1;
    }

    BpeModel loaded =
        std::move(
            loaded_result.Value()
        );

    /*
     * Structural persistence.
     */

    Expect(
        loaded.VocabularySize() ==
        original_vocab_size,
        "Vocabulary size survives disk round trip"
    );

    Expect(
        loaded.RuleCount() ==
        original_rule_count,
        "Rule count survives disk round trip"
    );

    Expect(
        SameVocabulary(
            original,
            loaded
        ),
        "Exact vocabulary IDs survive disk round trip"
    );

    Expect(
        original.GetRules() ==
        loaded.GetRules(),
        "Exact merge rules survive disk round trip"
    );

    /*
     * Inference equivalence.
     */

    bool all_encodings_equal = true;

    for (usize i = 0;
         i < inference_inputs.size();
         ++i)
    {
        auto encoded =
            loaded.Encode(
                inference_inputs[i]
            );

        if (encoded.Failed() ||
            i >= expected_encodings.size() ||
            encoded.Value() !=
                expected_encodings[i])
        {
            all_encodings_equal =
                false;

            break;
        }
    }

    Expect(
        all_encodings_equal,
        "All inference IDs identical after reload"
    );

    /*
     * Individual language checks make failures
     * easier to diagnose.
     */

    Expect(
        SameEncoding(
            original,
            loaded,
            "hello"
        ),
        "English encoding preserved"
    );

    Expect(
        SameEncoding(
            original,
            loaded,
            "தமிழ்"
        ),
        "Tamil encoding preserved"
    );

    Expect(
        SameEncoding(
            original,
            loaded,
            "hello தமிழ்"
        ),
        "Mixed-language encoding preserved"
    );

    /*
     * Display loaded inference for manual
     * inspection.
     */

    std::cout
        << "\n=== LOADED MODEL INFERENCE ===\n";

    for (const auto& input :
         inference_inputs)
    {
        PrintEncoding(
            loaded,
            input
        );
    }

    /*
     * Cleanup.
     */

    std::filesystem::remove(
        path,
        ec
    );

    Expect(
        !std::filesystem::exists(path),
        "Temporary model file removed"
    );

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
