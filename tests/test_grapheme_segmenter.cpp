#include <string>
#include <vector>

#include "unicode/grapheme_segmenter.hpp"
#include "test_framework.hpp"

using namespace qualix::unicode;
using namespace qualix::test;

static std::vector<std::string> Split(const std::string& text)
{
    auto result = GraphemeSegmenter::Segment(text);

    if (result.Failed())
        return {};

    std::vector<std::string> output;

    for (const auto& grapheme : result.Value())
        output.emplace_back(grapheme.View(text));

    return output;
}

int main()
{
    {
        auto g = Split("ABC");

        Expect(g.size() == 3, "ASCII three graphemes");
        Expect(g[0] == "A", "ASCII A");
        Expect(g[1] == "B", "ASCII B");
        Expect(g[2] == "C", "ASCII C");
    }

    {
        auto g = Split("க்");

        Expect(g.size() == 1, "Tamil KA virama one grapheme");
        Expect(g[0] == "க்", "Tamil KA virama preserved");
    }

    {
        auto g = Split("கி");

        Expect(g.size() == 1, "Tamil KI one grapheme");
        Expect(g[0] == "கி", "Tamil KI preserved");
    }

    {
        auto g = Split("e\u0301");

        Expect(g.size() == 1, "Combining accent one grapheme");
        Expect(g[0] == "e\u0301", "Combining accent preserved");
    }

    {
        auto g = Split("👍🏽");

        Expect(g.size() == 1, "Skin tone emoji one grapheme");
        Expect(g[0] == "👍🏽", "Skin tone emoji preserved");
    }

    {
        auto g = Split("🇮🇳");

        Expect(g.size() == 1, "India flag one grapheme");
        Expect(g[0] == "🇮🇳", "India flag preserved");
    }

    {
        auto g = Split("👨‍👩‍👧‍👦");

        Expect(g.size() == 1, "Family emoji one grapheme");
        Expect(g[0] == "👨‍👩‍👧‍👦", "Family emoji preserved");
    }

    {
        auto g = Split("क्ष");

        Expect(g.size() == 1, "Indic conjunct one grapheme");
        Expect(g[0] == "क्ष", "Indic conjunct preserved");
    }

    {
        auto g = Split("A👍🏽க்B");

        Expect(g.size() == 4, "Mixed text four graphemes");
        Expect(g[0] == "A", "Mixed A");
        Expect(g[1] == "👍🏽", "Mixed emoji");
        Expect(g[2] == "க்", "Mixed Tamil");
        Expect(g[3] == "B", "Mixed B");
    }

    {
        auto result = GraphemeSegmenter::Segment("");

        Expect(result.Ok(), "Empty input valid");
        Expect(result.Value().empty(), "Empty input no graphemes");
    }

    {
        std::string invalid("\x80", 1);

        auto result = GraphemeSegmenter::Segment(invalid);

        Expect(result.Failed(), "Invalid UTF-8 rejected");
    }

    return Summary();
}
