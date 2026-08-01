#include <string_view>

#include "pretokenizer/span.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::pretokenizer;
using namespace qualix::test;

int main()
{
    Span empty{};

    Expect(
        empty.byte_start == 0,
        "Default byte start"
    );

    Expect(
        empty.byte_length == 0,
        "Default byte length"
    );

    Expect(
        empty.grapheme_start == 0,
        "Default grapheme start"
    );

    Expect(
        empty.grapheme_count == 0,
        "Default grapheme count"
    );

    Expect(
        empty.type == SpanType::Unknown,
        "Default span type"
    );

    Expect(
        empty.policy == SpanPolicy::Splittable,
        "Default span policy"
    );

    Expect(
        empty.ByteEnd() == 0,
        "Default byte end"
    );

    Expect(
        empty.GraphemeEnd() == 0,
        "Default grapheme end"
    );

    Expect(
        empty.Empty(),
        "Default span empty"
    );

    Expect(
        !empty.Protected(),
        "Default span not protected"
    );

    std::string_view text =
        "Hello தமிழ்";

    Span hello{
        0,
        5,
        0,
        5,
        SpanType::Word,
        SpanPolicy::Splittable
    };

    Expect(
        hello.View(text) == "Hello",
        "ASCII span view"
    );

    Expect(
        hello.ByteEnd() == 5,
        "ASCII byte end"
    );

    Expect(
        hello.GraphemeEnd() == 5,
        "ASCII grapheme end"
    );

    Expect(
        hello.type == SpanType::Word,
        "ASCII word type"
    );

    Span tamil{
        6,
        std::string_view("தமிழ்").size(),
        6,
        3,
        SpanType::Word,
        SpanPolicy::Protected
    };

    Expect(
        tamil.View(text) == "தமிழ்",
        "Tamil span preserved"
    );

    Expect(
        tamil.grapheme_count == 3,
        "Tamil grapheme count"
    );

    Expect(
        tamil.Protected(),
        "Protected span"
    );

    Span invalid_start{
        100,
        1,
        0,
        0,
        SpanType::Other,
        SpanPolicy::Splittable
    };

    Expect(
        invalid_start.View(text).empty(),
        "Reject invalid byte start"
    );

    Span invalid_length{
        3,
        100,
        0,
        0,
        SpanType::Other,
        SpanPolicy::Splittable
    };

    Expect(
        invalid_length.View(text).empty(),
        "Reject invalid byte length"
    );

    Expect(
        ToString(SpanType::Word) == "Word",
        "Word type string"
    );

    Expect(
        ToString(SpanType::Url) == "Url",
        "URL type string"
    );

    Expect(
        ToString(SpanType::Currency) == "Currency",
        "Currency type string"
    );

    return Summary();
}
