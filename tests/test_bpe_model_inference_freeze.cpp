#include <iostream>
#include <string>

#include "bpe/model_trainer.hpp"
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

bool RoundTrip(
    BpeModel& model,
    const std::string& input
)
{
    const usize before =
        model.VocabularySize();

    auto encoded =
        model.Encode(input);

    if (encoded.Failed())
        return false;

    if (model.VocabularySize() != before)
        return false;

    auto decoded =
        model.Decode(
            encoded.Value()
        );

    return
        decoded.Ok() &&
        decoded.Value() == input &&
        model.VocabularySize() == before;
}

} // namespace

int main()
{
    const TrainerConfig config{
        128,
        2
    };

    const std::string corpus =
        "hello hello hello "
        "token token token "
        "தமிழ் தமிழ் தமிழ் "
        "namma namma namma";

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

    BpeModel& model =
        trained.Value();

    Expect(
        model.GetVocabulary().
            HasByteFallback(),
        "Trained model contains byte fallback"
    );

    const usize frozen_size =
        model.VocabularySize();

    /*
     * Learned input.
     */
    Expect(
        RoundTrip(
            model,
            "hello"
        ),
        "Learned English round trip without growth"
    );

    /*
     * Unknown ASCII.
     */
    Expect(
        RoundTrip(
            model,
            "xyz!"
        ),
        "OOV ASCII round trip without growth"
    );

    /*
     * Tanglish-style text.
     */
    Expect(
        RoundTrip(
            model,
            "namma epdi irukkom?"
        ),
        "Tanglish OOV round trip without growth"
    );

    /*
     * Unicode script absent from training.
     */
    Expect(
        RoundTrip(
            model,
            "नमस्ते"
        ),
        "Unknown Unicode script round trip"
    );

    /*
     * Emoji absent from training.
     */
    Expect(
        RoundTrip(
            model,
            "🧠🚀"
        ),
        "Unknown emoji round trip"
    );

    /*
     * Mixed learned + completely unseen content.
     */
    Expect(
        RoundTrip(
            model,
            "hello தமிழ் 🧠 नमस्ते!"
        ),
        "Mixed learned and OOV round trip"
    );

    Expect(
        model.VocabularySize() ==
            frozen_size,
        "Vocabulary remains globally frozen"
    );

    /*
     * Repeated OOV inference must remain stable.
     */
    auto first =
        model.Encode(
            "brand-new-OOV"
        );

    const usize after_first =
        model.VocabularySize();

    auto second =
        model.Encode(
            "brand-new-OOV"
        );

    Expect(
        first.Ok() &&
        second.Ok() &&
        first.Value() ==
            second.Value(),
        "Repeated OOV encoding is deterministic"
    );

    Expect(
        after_first == frozen_size &&
        model.VocabularySize() ==
            frozen_size,
        "Repeated OOV never mutates vocabulary"
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
