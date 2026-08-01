#include <string>
#include <string_view>
#include <vector>

#include "pretokenizer/grapheme_classifier.hpp"
#include "unicode/grapheme_segmenter.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::unicode;
using namespace qualix::pretokenizer;
using namespace qualix::test;

struct ExpectedGrapheme
{
    std::string_view text;
    GraphemeClass type;
};

static void CheckSequence(
    const std::string& input,
    const std::vector<ExpectedGrapheme>& expected,
    std::string_view name
)
{
    auto result =
        GraphemeSegmenter::Segment(input);

    Expect(
        result.Ok(),
        std::string(name) + " segmentation"
    );

    if (result.Failed())
        return;

    const auto& graphemes =
        result.Value();

    Expect(
        graphemes.size() == expected.size(),
        std::string(name) + " grapheme count"
    );

    if (graphemes.size() != expected.size())
        return;

    for (usize i = 0;
         i < graphemes.size();
         ++i)
    {
        const auto view =
            graphemes[i].View(input);

        Expect(
            view == expected[i].text,
            std::string(name) +
                " grapheme " +
                std::to_string(i)
        );

        Expect(
            GraphemeClassifier::Classify(view) ==
                expected[i].type,
            std::string(name) +
                " class " +
                std::to_string(i)
        );
    }
}

int main()
{
    CheckSequence(
        "Hello",
        {
            {"H", GraphemeClass::Letter},
            {"e", GraphemeClass::Letter},
            {"l", GraphemeClass::Letter},
            {"l", GraphemeClass::Letter},
            {"o", GraphemeClass::Letter}
        },
        "ASCII"
    );

    CheckSequence(
        "தமிழ்",
        {
            {"த", GraphemeClass::Letter},
            {"மி", GraphemeClass::Letter},
            {"ழ்", GraphemeClass::Letter}
        },
        "Tamil"
    );

    CheckSequence(
        "क्ष",
        {
            {"क्ष", GraphemeClass::Letter}
        },
        "Devanagari conjunct"
    );

    CheckSequence(
        "中文",
        {
            {"中", GraphemeClass::Letter},
            {"文", GraphemeClass::Letter}
        },
        "CJK"
    );

    CheckSequence(
        "한국어",
        {
            {"한", GraphemeClass::Letter},
            {"국", GraphemeClass::Letter},
            {"어", GraphemeClass::Letter}
        },
        "Korean"
    );

    CheckSequence(
        "العربية",
        {
            {"ا", GraphemeClass::Letter},
            {"ل", GraphemeClass::Letter},
            {"ع", GraphemeClass::Letter},
            {"ر", GraphemeClass::Letter},
            {"ب", GraphemeClass::Letter},
            {"ي", GraphemeClass::Letter},
            {"ة", GraphemeClass::Letter}
        },
        "Arabic"
    );

    CheckSequence(
        "e\u0301",
        {
            {"e\u0301", GraphemeClass::Letter}
        },
        "Decomposed Latin"
    );

    CheckSequence(
        "123",
        {
            {"1", GraphemeClass::Number},
            {"2", GraphemeClass::Number},
            {"3", GraphemeClass::Number}
        },
        "Numbers"
    );

    CheckSequence(
        " \t\n",
        {
            {" ", GraphemeClass::Whitespace},
            {"\t", GraphemeClass::Whitespace},
            {"\n", GraphemeClass::Whitespace}
        },
        "Whitespace"
    );

    CheckSequence(
        "!。",
        {
            {"!", GraphemeClass::Punctuation},
            {"。", GraphemeClass::Punctuation}
        },
        "Punctuation"
    );

    CheckSequence(
        "₹∑∞",
        {
            {"₹", GraphemeClass::Symbol},
            {"∑", GraphemeClass::Symbol},
            {"∞", GraphemeClass::Symbol}
        },
        "Symbols"
    );

    CheckSequence(
        "👍🏽❤️👨‍👩‍👧‍👦",
        {
            {"👍🏽", GraphemeClass::Emoji},
            {"❤️", GraphemeClass::Emoji},
            {"👨‍👩‍👧‍👦", GraphemeClass::Emoji}
        },
        "Emoji"
    );

    CheckSequence(
        "A தமிழ் 7 ₹ 👍🏽!",
        {
            {"A", GraphemeClass::Letter},
            {" ", GraphemeClass::Whitespace},
            {"த", GraphemeClass::Letter},
            {"மி", GraphemeClass::Letter},
            {"ழ்", GraphemeClass::Letter},
            {" ", GraphemeClass::Whitespace},
            {"7", GraphemeClass::Number},
            {" ", GraphemeClass::Whitespace},
            {"₹", GraphemeClass::Symbol},
            {" ", GraphemeClass::Whitespace},
            {"👍🏽", GraphemeClass::Emoji},
            {"!", GraphemeClass::Punctuation}
        },
        "Mixed multilingual"
    );

    {
        std::string invalid("\x80", 1);

        auto result =
            GraphemeSegmenter::Segment(invalid);

        Expect(
            result.Failed(),
            "Invalid UTF8 rejected before classification"
        );
    }

    return Summary();
}
