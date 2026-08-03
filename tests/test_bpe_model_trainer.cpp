#include <iostream>
#include <string>
#include <vector>

#include "bpe/model_trainer.hpp"

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

} // namespace

int main()
{
    /*
     * Empty corpus.
     */
    {
        TrainerConfig config;

        config.max_merges = 10;
        config.min_frequency = 2;

        auto result =
            BpeModelTrainer::Train(
                "",
                config
            );

        Expect(
            result.Ok(),
            "Empty input training succeeds"
        );

        if (result.Ok())
        {
            Expect(
                result.Value().Empty(),
                "Empty input produces empty model"
            );
        }
    }

    /*
     * Basic repeated text.
     *
     * Repetition must provide enough pair
     * frequency for at least one BPE merge.
     */
    {
        TrainerConfig config;

        config.max_merges = 16;
        config.min_frequency = 2;

        auto result =
            BpeModelTrainer::Train(
                "hello hello hello hello",
                config
            );

        Expect(
            result.Ok(),
            "Basic corpus training succeeds"
        );

        if (result.Ok())
        {
            auto& model =
                result.Value();

            Expect(
                model.VocabularySize() > 0,
                "Vocabulary automatically populated"
            );

            Expect(
                model.RuleCount() > 0,
                "Repeated corpus learns BPE rules"
            );

            auto encoded =
                model.Encode(
                    "hello"
                );

            Expect(
                encoded.Ok(),
                "Trained model encodes known text"
            );

            if (encoded.Ok())
            {
                Expect(
                    !encoded.Value().empty(),
                    "Known text produces symbols"
                );
            }
        }
    }

    /*
     * Training and inference must share the
     * exact same vocabulary.
     */
    {
        TrainerConfig config;

        config.max_merges = 8;
        config.min_frequency = 2;

        auto result =
            BpeModelTrainer::Train(
                "abab abab abab",
                config
            );

        Expect(
            result.Ok(),
            "Shared vocabulary training succeeds"
        );

        if (result.Ok())
        {
            auto& model =
                result.Value();

            const auto a =
                model.GetVocabulary().Find(
                    "a"
                );

            const auto b =
                model.GetVocabulary().Find(
                    "b"
                );

            Expect(
                a.has_value(),
                "Base symbol a stored"
            );

            Expect(
                b.has_value(),
                "Base symbol b stored"
            );

            auto encoded =
                model.Encode(
                    "abab"
                );

            Expect(
                encoded.Ok(),
                "Inference uses trained vocabulary"
            );
        }
    }

    /*
     * Learned merged symbols must physically
     * exist in the vocabulary.
     */
    {
        TrainerConfig config;

        config.max_merges = 1;
        config.min_frequency = 2;

        auto result =
            BpeModelTrainer::Train(
                "aaaa",
                config
            );

        Expect(
            result.Ok(),
            "Merged vocabulary training succeeds"
        );

        if (result.Ok())
        {
            const auto& model =
                result.Value();

            Expect(
                model.RuleCount() == 1,
                "Exactly one merge learned"
            );

            if (!model.GetRules().empty())
            {
                const auto merged =
                    model.GetRules()[0]
                        .merged_symbol;

                Expect(
                    model.GetVocabulary()
                        .Contains(merged),
                    "Merged symbol registered in vocabulary"
                );

                const auto text =
                    model.GetVocabulary()
                        .Find(merged);

                Expect(
                    text.has_value(),
                    "Merged symbol text resolvable"
                );

                if (text.has_value())
                {
                    Expect(
                        *text == "aa",
                        "Merged symbol stores concatenated text"
                    );
                }
            }
        }
    }

    /*
     * max_merges = 0 must still build the base
     * vocabulary, but learn no BPE rules.
     */
    {
        TrainerConfig config;

        config.max_merges = 0;
        config.min_frequency = 1;

        auto result =
            BpeModelTrainer::Train(
                "qualix",
                config
            );

        Expect(
            result.Ok(),
            "Zero merge training succeeds"
        );

        if (result.Ok())
        {
            const auto& model =
                result.Value();

            Expect(
                model.VocabularySize() > 0,
                "Zero merge model keeps base vocabulary"
            );

            Expect(
                model.RuleCount() == 0,
                "Zero merge model learns no rules"
            );
        }
    }

    /*
     * min_frequency must be respected.
     */
    {
        TrainerConfig config;

        config.max_merges = 100;
        config.min_frequency = 1000;

        auto result =
            BpeModelTrainer::Train(
                "abcdef",
                config
            );

        Expect(
            result.Ok(),
            "High minimum frequency training succeeds"
        );

        if (result.Ok())
        {
            Expect(
                result.Value().RuleCount() == 0,
                "High minimum frequency prevents merges"
            );
        }
    }

    /*
     * Multilingual corpus.
     */
    {
        TrainerConfig config;

        config.max_merges = 32;
        config.min_frequency = 2;

        const std::string corpus =
            "தமிழ் தமிழ் தமிழ் "
            "English English English "
            "हिन्दी हिन्दी हिन्दी";

        auto result =
            BpeModelTrainer::Train(
                corpus,
                config
            );

        Expect(
            result.Ok(),
            "Multilingual training succeeds"
        );

        if (result.Ok())
        {
            auto& model =
                result.Value();

            Expect(
                model.VocabularySize() > 0,
                "Multilingual vocabulary populated"
            );

            auto tamil =
                model.Encode(
                    "தமிழ்"
                );

            Expect(
                tamil.Ok(),
                "Tamil inference succeeds"
            );

            auto english =
                model.Encode(
                    "English"
                );

            Expect(
                english.Ok(),
                "English inference succeeds"
            );

            auto hindi =
                model.Encode(
                    "हिन्दी"
                );

            Expect(
                hindi.Ok(),
                "Hindi inference succeeds"
            );
        }
    }

    /*
     * Model rules must remain usable through
     * EncodeSymbols().
     */
    {
        TrainerConfig config;

        config.max_merges = 4;
        config.min_frequency = 2;

        auto result =
            BpeModelTrainer::Train(
                "abababab",
                config
            );

        Expect(
            result.Ok(),
            "Symbol inference model training succeeds"
        );

        if (result.Ok())
        {
            auto& model =
                result.Value();

            const auto a =
                model.GetVocabulary().Find(
                    "a"
                );

            const auto b =
                model.GetVocabulary().Find(
                    "b"
                );

            if (a.has_value() &&
                b.has_value())
            {
                const std::vector<SymbolId>
                    raw{
                        *a,
                        *b,
                        *a,
                        *b
                    };

                const auto encoded =
                    model.EncodeSymbols(
                        raw
                    );

                Expect(
                    !encoded.empty(),
                    "EncodeSymbols uses learned rules"
                );

                Expect(
                    encoded.size() <=
                        raw.size(),
                    "BPE does not expand symbol sequence"
                );
            }
            else
            {
                Expect(
                    false,
                    "Required base symbols exist"
                );
            }
        }
    }

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

    return failed == 0 ? 0 : 1;
}
