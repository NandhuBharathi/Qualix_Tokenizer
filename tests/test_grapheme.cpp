#include <string_view>

#include "unicode/grapheme.hpp"
#include "test_framework.hpp"

using namespace qualix::unicode;
using namespace qualix::test;

int main()
{
    Grapheme empty{};

    Expect(empty.byte_start == 0, "Default byte start");
    Expect(empty.byte_length == 0, "Default byte length");
    Expect(empty.ByteEnd() == 0, "Default byte end");
    Expect(empty.Empty(), "Default grapheme empty");

    std::string_view ascii = "Hello";

    Grapheme ascii_g{1, 3};

    Expect(ascii_g.byte_start == 1, "ASCII byte start");
    Expect(ascii_g.byte_length == 3, "ASCII byte length");
    Expect(ascii_g.ByteEnd() == 4, "ASCII byte end");
    Expect(!ascii_g.Empty(), "ASCII grapheme not empty");
    Expect(ascii_g.View(ascii) == "ell", "ASCII grapheme view");

    std::string_view tamil = "க்";

    Grapheme tamil_g{0, tamil.size()};

    Expect(!tamil_g.Empty(), "Tamil grapheme not empty");
    Expect(tamil_g.ByteEnd() == tamil.size(), "Tamil byte end");
    Expect(tamil_g.View(tamil) == "க்", "Tamil grapheme preserved");

    std::string_view emoji = "👍🏽";

    Grapheme emoji_g{0, emoji.size()};

    Expect(emoji_g.View(emoji) == "👍🏽", "Emoji grapheme preserved");

    Grapheme invalid_start{100, 1};
    Expect(invalid_start.View(ascii).empty(), "Reject invalid start");

    Grapheme invalid_length{3, 100};
    Expect(invalid_length.View(ascii).empty(), "Reject invalid length");

    return Summary();
}
