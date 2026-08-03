
#include <iostream>
#include <string>
#include <vector>

#include "bpe/model_trainer.hpp"

using namespace qualix;
using namespace qualix::bpe;

struct Case
{
    const char* name;
    std::string text;
};

int main()
{
    /*
     * Train on deliberately limited data.
     * Most adversarial inputs below therefore
     * exercise byte fallback during inference.
     */
    const std::string corpus =
        "hello hello world world "
        "தமிழ் தமிழ் வணக்கம் வணக்கம் "
        "namma namma tokenizer tokenizer";

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

    const usize frozen_size =
        model.VocabularySize();

    const std::vector<Case> cases{
        {
            "empty",
            ""
        },
        {
            "spaces",
            "     "
        },
        {
            "newlines-tabs",
            "\n\t\r\n\t"
        },
        {
            "repeated-ascii",
            std::string(4096, 'x')
        },
        {
            "tamil",
            "தமிழ் வணக்கம் உலகம் ழ ஃ"
        },
        {
            "tamil-oov",
            "ஞாயிற்றுக்கிழமை அழகானது"
        },
        {
            "emoji",
            "🧠🚀🔥❤️✨"
        },
        {
            "emoji-zwj",
            "👨‍👩‍👧‍👦 👩‍💻 🧑‍🚀"
        },
        {
            "cjk",
            "你好世界 日本語 한국어"
        },
        {
            "mixed",
            "Hello தமிழ் 你好 🧠 123 !@#$%^&*()"
        },
        {
            "tanglish",
            "namma tokenizer semma fast-ah irukkanum!"
        },
        {
            "combining",
            "e\xCC\x81 a\xCC\x88 n\xCC\x83"
        },
        {
            "rtl",
            "مرحبا بالعالم שלום עולם"
        },
        {
            "rare-unicode",
            "𐍈 𓂀 𝕼 ℵ ∑ ∫ √ ∞"
        },
        {
            "long-mixed",
            std::string(2048, 'A') +
            "தமிழ்🧠你好" +
            std::string(2048, 'Z')
        }
    };

    usize passed = 0;

    for (const auto& test : cases)
    {
        const usize before =
            model.VocabularySize();

        auto encoded =
            model.Encode(
                test.text
            );

        if (encoded.Failed())
        {
            std::cout
                << "[FAIL] "
                << test.name
                << " encode\n";
            return 1;
        }

        /*
         * Inference must never grow vocabulary.
         */
        if (model.VocabularySize() != before ||
            model.VocabularySize() != frozen_size)
        {
            std::cout
                << "[FAIL] "
                << test.name
                << " vocabulary mutated\n";
            return 1;
        }

        auto decoded =
            model.Decode(
                encoded.Value()
            );

        if (decoded.Failed())
        {
            std::cout
                << "[FAIL] "
                << test.name
                << " decode\n";
            return 1;
        }

        /*
         * Exact byte-for-byte round trip.
         */
        if (decoded.Value() != test.text)
        {
            std::cout
                << "[FAIL] "
                << test.name
                << " round-trip\n";
            return 1;
        }

        ++passed;
    }

    /*
     * Invalid UTF-8 must still be rejected.
     */
    const std::string invalid(
        "\xC0\xAF",
        2
    );

    const usize before_invalid =
        model.VocabularySize();

    auto bad =
        model.Encode(
            invalid
        );

    if (!bad.Failed())
    {
        std::cout
            << "[FAIL] invalid UTF-8 accepted\n";
        return 1;
    }

    if (model.VocabularySize() !=
        before_invalid)
    {
        std::cout
            << "[FAIL] invalid UTF-8 mutated vocabulary\n";
        return 1;
    }

    std::cout
        << "[PASS] "
        << passed
        << "/"
        << cases.size()
        << " adversarial round-trips\n"
        << "[PASS] inference vocabulary frozen\n"
        << "[PASS] OOV byte fallback lossless\n"
        << "[PASS] invalid UTF-8 rejected\n"
        << "Vocabulary : "
        << model.VocabularySize()
        << '\n'
        << "Rules      : "
        << model.RuleCount()
        << '\n';

    return 0;
}
