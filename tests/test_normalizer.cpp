#include <string>

#include "unicode/normalizer.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::unicode;
using namespace qualix::test;

int main()
{
    // ---------------------------------------------------------
    // None
    // ---------------------------------------------------------

    {
        auto result = Normalizer::Normalize(
            "Hello",
            NormalizationForm::None
        );

        Expect(result.Ok(), "None ASCII normalize");
        Expect(result.Value() == "Hello", "None ASCII preserved");
    }

    {
        const std::string input =
            "Hello தமிழ் हिन्दी 中文 한국어 العربية";

        auto result = Normalizer::Normalize(
            input,
            NormalizationForm::None
        );

        Expect(result.Ok(), "None multilingual normalize");
        Expect(result.Value() == input, "None multilingual preserved");
    }

    {
        const std::string input = "👍🏽❤️🇮🇳👨‍👩‍👧‍👦";

        auto result = Normalizer::Normalize(
            input,
            NormalizationForm::None
        );

        Expect(result.Ok(), "None emoji normalize");
        Expect(result.Value() == input, "None emoji preserved");
    }

    {
        const std::string input = "e\u0301";

        auto result = Normalizer::Normalize(
            input,
            NormalizationForm::None
        );

        Expect(result.Ok(), "None decomposed input");
        Expect(result.Value() == input, "None decomposition preserved");
    }

    {
        auto result = Normalizer::Normalize(
            "Qualix",
            NormalizationForm::None
        );

        Expect(result.Ok(), "None reports normalized");
    }

    // ---------------------------------------------------------
    // Invalid UTF-8
    // ---------------------------------------------------------

    {
        const std::string invalid("\xC0\xAF", 2);

        auto result = Normalizer::Normalize(
            invalid,
            NormalizationForm::NFD
        );

        Expect(result.Failed(), "Invalid UTF-8 rejected");
        Expect(!result.Ok(), "Invalid UTF-8 not normalized");
    }

    // ---------------------------------------------------------
    // NFD
    // ---------------------------------------------------------

    {
        auto result = Normalizer::Normalize(
            "\u00E9",
            NormalizationForm::NFD
        );

        Expect(result.Ok(), "NFD Latin acute");
        Expect(
            result.Value() == "e\u0301",
            "NFD Latin acute decomposition"
        );
    }

    {
        auto result = Normalizer::Normalize(
            "\u00C5",
            NormalizationForm::NFD
        );

        Expect(result.Ok(), "NFD Latin ring");
        Expect(
            result.Value() == "A\u030A",
            "NFD Latin ring decomposition"
        );
    }

    {
        auto result = Normalizer::Normalize(
            "\u01FA",
            NormalizationForm::NFD
        );

        Expect(result.Ok(), "NFD recursive decomposition");
        Expect(
            result.Value() == "A\u030A\u0301",
            "NFD recursive result"
        );
    }

    {
        auto result = Normalizer::Normalize(
            "\uAC00",
            NormalizationForm::NFD
        );

        Expect(result.Ok(), "NFD Hangul GA");
        Expect(
            result.Value() == "\u1100\u1161",
            "NFD Hangul GA decomposition"
        );
    }

    {
        auto result = Normalizer::Normalize(
            "\uAC01",
            NormalizationForm::NFD
        );

        Expect(result.Ok(), "NFD Hangul GAG");
        Expect(
            result.Value() == "\u1100\u1161\u11A8",
            "NFD Hangul GAG decomposition"
        );
    }

    {
        auto result = Normalizer::Normalize(
            "a\u0315\u0300",
            NormalizationForm::NFD
        );

        Expect(result.Ok(), "NFD canonical reorder");
        Expect(
            result.Value() == "a\u0300\u0315",
            "NFD canonical combining order"
        );
    }

    {
        const std::string input =
            "தமிழ் \u00E9 한국어";

        auto result = Normalizer::Normalize(
            input,
            NormalizationForm::NFD
        );

        Expect(result.Ok(), "NFD multilingual");
        Expect(
            result.Value() ==
                "தமிழ் e\u0301 한국어",
            "NFD output reports normalized"
        );
    }

    // ---------------------------------------------------------
    // NFC
    // ---------------------------------------------------------

    {
        auto result = Normalizer::Normalize(
            "e\u0301",
            NormalizationForm::NFC
        );

        Expect(result.Ok(), "NFC Latin acute");
        Expect(
            result.Value() == "\u00E9",
            "NFC Latin acute composition"
        );
    }

    {
        auto result = Normalizer::Normalize(
            "A\u030A",
            NormalizationForm::NFC
        );

        Expect(result.Ok(), "NFC Latin ring");
        Expect(
            result.Value() == "\u00C5",
            "NFC Latin ring composition"
        );
    }

    {
        auto result = Normalizer::Normalize(
            "A\u030A\u0301",
            NormalizationForm::NFC
        );

        Expect(result.Ok(), "NFC recursive composition");
        Expect(
            result.Value() == "\u01FA",
            "NFC recursive composition result"
        );
    }

    {
        auto result = Normalizer::Normalize(
            "\u1100\u1161",
            NormalizationForm::NFC
        );

        Expect(result.Ok(), "NFC Hangul LV");
        Expect(
            result.Value() == "\uAC00",
            "NFC Hangul LV composition"
        );
    }

    {
        auto result = Normalizer::Normalize(
            "\u1100\u1161\u11A8",
            NormalizationForm::NFC
        );

        Expect(result.Ok(), "NFC Hangul LVT");
        Expect(
            result.Value() == "\uAC01",
            "NFC Hangul LVT composition"
        );
    }

    {
        const std::string input =
            "தமிழ் e\u0301 한국어";

        auto result = Normalizer::Normalize(
            input,
            NormalizationForm::NFC
        );

        Expect(result.Ok(), "NFC multilingual");
        Expect(
            result.Value() ==
                "தமிழ் \u00E9 한국어",
            "NFC output reports normalized"
        );
    }

    // ---------------------------------------------------------
    // NFKD
    // ---------------------------------------------------------

    {
        auto result = Normalizer::Normalize(
            "\u2460",
            NormalizationForm::NFKD
        );

        Expect(result.Ok(), "NFKD circled digit");
        Expect(
            result.Value() == "1",
            "NFKD circled digit decomposition"
        );
    }

    {
        auto result = Normalizer::Normalize(
            "\uFB01",
            NormalizationForm::NFKD
        );

        Expect(result.Ok(), "NFKD ligature");
        Expect(
            result.Value() == "fi",
            "NFKD ligature decomposition"
        );
    }

    {
        auto result = Normalizer::Normalize(
            "\uFF21",
            NormalizationForm::NFKD
        );

        Expect(result.Ok(), "NFKD fullwidth Latin");
        Expect(
            result.Value() == "A",
            "NFKD fullwidth Latin decomposition"
        );
    }

    {
        auto result = Normalizer::Normalize(
            "\u00E9",
            NormalizationForm::NFKD
        );

        Expect(result.Ok(), "NFKD canonical mapping");
        Expect(
            result.Value() == "e\u0301",
            "NFKD includes canonical decomposition"
        );
    }

    {
        auto result = Normalizer::Normalize(
            "\uAC01",
            NormalizationForm::NFKD
        );

        Expect(result.Ok(), "NFKD Hangul");
        Expect(
            result.Value() == "\u1100\u1161\u11A8",
            "NFKD Hangul decomposition"
        );
    }

    {
        auto result = Normalizer::Normalize(
            "\u212B",
            NormalizationForm::NFKD
        );

        Expect(result.Ok(), "NFKD recursive mapping");
        Expect(
            result.Value() == "A\u030A",
            "NFKD recursive canonical result"
        );
    }


    // ---------------------------------------------------------
    // NFKC
    // ---------------------------------------------------------

    {
        auto result = Normalizer::Normalize(
            "\u2460",
            NormalizationForm::NFKC
        );

        Expect(result.Ok(), "NFKC circled digit");
        Expect(
            result.Value() == "1",
            "NFKC circled digit composition"
        );
    }

    {
        auto result = Normalizer::Normalize(
            "\uFB01",
            NormalizationForm::NFKC
        );

        Expect(result.Ok(), "NFKC ligature");
        Expect(
            result.Value() == "fi",
            "NFKC ligature composition"
        );
    }

    {
        auto result = Normalizer::Normalize(
            "\uFF21",
            NormalizationForm::NFKC
        );

        Expect(result.Ok(), "NFKC fullwidth Latin");
        Expect(
            result.Value() == "A",
            "NFKC fullwidth Latin composition"
        );
    }

    {
        auto result = Normalizer::Normalize(
            "e\u0301",
            NormalizationForm::NFKC
        );

        Expect(result.Ok(), "NFKC canonical composition");
        Expect(
            result.Value() == "\u00E9",
            "NFKC Latin acute composed"
        );
    }

    {
        auto result = Normalizer::Normalize(
            "\u212B",
            NormalizationForm::NFKC
        );

        Expect(result.Ok(), "NFKC recursive mapping");
        Expect(
            result.Value() == "\u00C5",
            "NFKC recursive composition"
        );
    }

    {
        auto result = Normalizer::Normalize(
            "\u1100\u1161\u11A8",
            NormalizationForm::NFKC
        );

        Expect(result.Ok(), "NFKC Hangul");
        Expect(
            result.Value() == "\uAC01",
            "NFKC Hangul composition"
        );
    }

    return Summary();
}
