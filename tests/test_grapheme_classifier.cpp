#include "pretokenizer/grapheme_classifier.hpp"
#include "test_framework.hpp"

using namespace qualix::pretokenizer;
using namespace qualix::test;

int main()
{
    Expect(
        GraphemeClassifier::Classify("A") ==
            GraphemeClass::Letter,
        "Latin letter"
    );

    Expect(
        GraphemeClassifier::Classify("é") ==
            GraphemeClass::Letter,
        "Composed Latin letter"
    );

    Expect(
        GraphemeClassifier::Classify("e\u0301") ==
            GraphemeClass::Letter,
        "Decomposed Latin letter"
    );

    Expect(
        GraphemeClassifier::Classify("க்") ==
            GraphemeClass::Letter,
        "Tamil grapheme"
    );

    Expect(
        GraphemeClassifier::Classify("क्ष") ==
            GraphemeClass::Letter,
        "Devanagari conjunct"
    );

    Expect(
        GraphemeClassifier::Classify("中") ==
            GraphemeClass::Letter,
        "CJK letter"
    );

    Expect(
        GraphemeClassifier::Classify("7") ==
            GraphemeClass::Number,
        "ASCII number"
    );

    Expect(
        GraphemeClassifier::Classify("௭") ==
            GraphemeClass::Number,
        "Tamil number"
    );

    Expect(
        GraphemeClassifier::Classify(" ") ==
            GraphemeClass::Whitespace,
        "ASCII space"
    );

    Expect(
        GraphemeClassifier::Classify("\n") ==
            GraphemeClass::Whitespace,
        "Newline whitespace"
    );

    Expect(
        GraphemeClassifier::Classify("!") ==
            GraphemeClass::Punctuation,
        "ASCII punctuation"
    );

    Expect(
        GraphemeClassifier::Classify("。") ==
            GraphemeClass::Punctuation,
        "CJK punctuation"
    );

    Expect(
        GraphemeClassifier::Classify("₹") ==
            GraphemeClass::Symbol,
        "Currency symbol"
    );

    Expect(
        GraphemeClassifier::Classify("∑") ==
            GraphemeClass::Symbol,
        "Math symbol"
    );

    Expect(
        GraphemeClassifier::Classify("👍🏽") ==
            GraphemeClass::Emoji,
        "Emoji skin tone"
    );

    Expect(
        GraphemeClassifier::Classify("❤️") ==
            GraphemeClass::Emoji,
        "Emoji variation sequence"
    );

    Expect(
        GraphemeClassifier::Classify("👨‍👩‍👧‍👦") ==
            GraphemeClass::Emoji,
        "Emoji ZWJ family"
    );

    Expect(
        GraphemeClassifier::Classify("") ==
            GraphemeClass::Other,
        "Empty grapheme"
    );

    Expect(
        GraphemeClassifier::Classify("\xFF") ==
            GraphemeClass::Other,
        "Invalid UTF8"
    );

    Expect(
        ToString(GraphemeClass::Letter) == "Letter",
        "Letter class string"
    );

    Expect(
        ToString(GraphemeClass::Emoji) == "Emoji",
        "Emoji class string"
    );

    return Summary();
}
