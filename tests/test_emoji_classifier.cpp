#include "pretokenizer/grapheme_classifier.hpp"
#include "pretokenizer/grapheme_class.hpp"
#include "test_framework.hpp"

using namespace qualix::pretokenizer;
using namespace qualix::test;

int main()
{
    Expect(
        GraphemeClassifier::Classify("👍") ==
        GraphemeClass::Emoji,
        "Default emoji presentation"
    );

    Expect(
        GraphemeClassifier::Classify("👍🏽") ==
        GraphemeClass::Emoji,
        "Emoji modifier sequence"
    );

    Expect(
        GraphemeClassifier::Classify("❤️") ==
        GraphemeClass::Emoji,
        "VS16 emoji sequence"
    );

    Expect(
        GraphemeClassifier::Classify("👨‍👩‍👧‍👦") ==
        GraphemeClass::Emoji,
        "Emoji ZWJ sequence"
    );

    Expect(
        GraphemeClassifier::Classify("🇮🇳") ==
        GraphemeClass::Emoji,
        "Regional indicator flag sequence"
    );

    Expect(
        GraphemeClassifier::Classify("©") ==
        GraphemeClass::Symbol,
        "Plain copyright symbol"
    );

    Expect(
        GraphemeClassifier::Classify("©️") ==
        GraphemeClass::Emoji,
        "Copyright VS16 emoji"
    );

    Expect(
        GraphemeClassifier::Classify("™") ==
        GraphemeClass::Symbol,
        "Plain trademark symbol"
    );

    Expect(
        GraphemeClassifier::Classify("™️") ==
        GraphemeClass::Emoji,
        "Trademark VS16 emoji"
    );

    Expect(
        GraphemeClassifier::Classify("A") ==
        GraphemeClass::Letter,
        "Latin letter unaffected"
    );

    Expect(
        GraphemeClassifier::Classify("தமிழ்") ==
        GraphemeClass::Letter,
        "Tamil grapheme unaffected"
    );

    Expect(
        GraphemeClassifier::Classify("₹") ==
        GraphemeClass::Symbol,
        "Currency symbol unaffected"
    );

    return Summary();
}
