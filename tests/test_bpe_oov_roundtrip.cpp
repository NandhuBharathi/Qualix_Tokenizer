
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "bpe/model.hpp"
#include "bpe/model_io.hpp"
#include "bpe/model_trainer.hpp"
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

bool RoundTripFrozen(
    BpeModel& model,
    std::string_view input
)
{
    const usize before =
        model.VocabularySize();

    auto encoded =
        model.Encode(input);

    if (encoded.Failed())
        return false;

    const usize after_encode =
        model.VocabularySize();

    if (after_encode != before)
        return false;

    auto decoded =
        model.Decode(
            encoded.Value()
        );

    if (decoded.Failed())
        return false;

    const usize after_decode =
        model.VocabularySize();

    return
        after_decode == before &&
        decoded.Value() == input;
}

} // namespace

int main()
{
    /*
     * Train on deliberately limited data.
     *
     * The inference inputs below contain symbols,
     * scripts and emoji that do not occur here.
     */
    const TrainerConfig config{
        128,
        2
    };

    const std::string corpus =
        "hello hello hello "
        "token token token "
        "தமிழ் தமிழ் தமிழ் "
        "namma namma";

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

    BpeModel& model =
        trained.Value();

    /*
     * Training must finalize the universal byte
     * fallback alphabet before the model enters
     * inference.
     */
    Expect(
        model.GetVocabulary().
            HasByteFallback(),
        "Trained model has byte fallback"
    );

    const usize frozen_size =
        model.VocabularySize();

    /*
     * Learned input remains lossless.
     */
    Expect(
        RoundTripFrozen(
            model,
            "hello"
        ),
        "Learned English round trip"
    );

    /*
     * Unseen ASCII.
     */
    Expect(
        RoundTripFrozen(
            model,
            "xyz"
        ),
        "OOV English round trip"
    );

    /*
     * Mixed learned + unseen ASCII.
     */
    Expect(
        RoundTripFrozen(
            model,
            "hello xyz!"
        ),
        "Mixed learned and OOV ASCII round trip"
    );

    /*
     * Tamil text containing unseen graphemes.
     */
    Expect(
        RoundTripFrozen(
            model,
            "வணக்கம்"
        ),
        "OOV Tamil round trip"
    );

    /*
     * Completely unseen emoji.
     */
    Expect(
        RoundTripFrozen(
            model,
            "🧠"
        ),
        "OOV emoji round trip"
    );

    /*
     * Multi-code-point emoji grapheme.
     */
    Expect(
        RoundTripFrozen(
            model,
            "👨‍👩‍👧‍👦"
        ),
        "OOV ZWJ emoji round trip"
    );

    /*
     * Emoji modifier sequence.
     */
    Expect(
        RoundTripFrozen(
            model,
            "👍🏽"
        ),
        "OOV emoji modifier round trip"
    );

    /*
     * Tanglish + Tamil + emoji + punctuation.
     */
    Expect(
        RoundTripFrozen(
            model,
            "namma தமிழ் super-aa இருக்கு 🧠!"
        ),
        "Mixed multilingual OOV round trip"
    );

    /*
     * Non-Tamil script unseen during training.
     */
    Expect(
        RoundTripFrozen(
            model,
            "こんにちは"
        ),
        "OOV Japanese round trip"
    );

    /*
     * Another unseen script.
     */
    Expect(
        RoundTripFrozen(
            model,
            "مرحبا"
        ),
        "OOV Arabic round trip"
    );

    /*
     * Vocabulary must remain exactly frozen after
     * all inference operations.
     */
    Expect(
        model.VocabularySize() ==
            frozen_size,
        "Inference vocabulary remains frozen"
    );

    /*
     * Persistence test.
     *
     * Save the already-trained frozen model,
     * reload it, and repeat OOV inference.
     */
    const auto path =
        std::filesystem::
            temp_directory_path() /
        "qualix_bpe_oov_roundtrip.qlxbpe";

    const Status saved =
        BpeModelIO::Save(
            model,
            path
        );

    Expect(
        saved.Ok(),
        "Frozen model save succeeds"
    );

    if (saved.Ok())
    {
        auto loaded =
            BpeModelIO::Load(
                path
            );

        Expect(
            loaded.Ok(),
            "Frozen model load succeeds"
        );

        if (loaded.Ok())
        {
            BpeModel& restored =
                loaded.Value();

            Expect(
                restored.GetVocabulary().
                    HasByteFallback(),
                "Loaded model preserves byte fallback"
            );

            const usize loaded_size =
                restored.VocabularySize();

            Expect(
                loaded_size ==
                    frozen_size,
                "Loaded vocabulary size preserved"
            );

            Expect(
                RoundTripFrozen(
                    restored,
                    "Completely unseen 🧠 வணக்கம்!"
                ),
                "Loaded model OOV round trip"
            );

            Expect(
                RoundTripFrozen(
                    restored,
                    "日本語 + தமிழ் + English 🚀"
                ),
                "Loaded multilingual OOV round trip"
            );

            Expect(
                restored.VocabularySize() ==
                    loaded_size,
                "Loaded inference vocabulary frozen"
            );
        }
    }

    std::error_code ec;

    std::filesystem::remove(
        path,
        ec
    );

    const usize failed =
        tests_run -
        tests_passed;

    std::cout
        << "\n================================\n"
        << "Tests Run    : "
        << tests_run
        << '\n'
        << "Tests Passed : "
        << tests_passed
        << '\n'
        << "Tests Failed : "
        << failed
        << '\n'
        << "================================\n";

    return
        failed == 0
            ? 0
            : 1;
}
