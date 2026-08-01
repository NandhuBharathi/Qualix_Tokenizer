#include "unicode/emoji_property.hpp"
#include "unicode/grapheme_property.hpp"
#include "test_framework.hpp"

using namespace qualix::unicode;
using namespace qualix::test;

int main()
{
    Expect(
        IsEmoji(0x1F44D),
        "Thumbs up Emoji"
    );

    Expect(
        IsEmojiPresentation(0x1F44D),
        "Thumbs up default emoji presentation"
    );

    Expect(
        IsEmojiModifier(0x1F3FD),
        "Medium skin tone modifier"
    );

    Expect(
        IsEmojiModifierBase(0x1F44D),
        "Thumbs up modifier base"
    );

    Expect(
        IsEmoji(0x00A9),
        "Copyright has Emoji property"
    );

    Expect(
        !IsEmojiPresentation(0x00A9),
        "Copyright not default emoji presentation"
    );

    Expect(
        IsEmoji(0x2122),
        "Trademark has Emoji property"
    );

    Expect(
        !IsEmojiPresentation(0x2122),
        "Trademark not default emoji presentation"
    );

    Expect(
        IsRegionalIndicator(0x1F1EE),
        "Regional indicator I"
    );

    Expect(
        IsRegionalIndicator(0x1F1F3),
        "Regional indicator N"
    );

    Expect(
        !IsRegionalIndicator(0x0041),
        "Latin A not regional indicator"
    );

    Expect(
        IsExtendedPictographic(0x1F44D),
        "Thumbs up extended pictographic"
    );

    Expect(
        IsExtendedPictographic(0x00A9),
        "Copyright extended pictographic"
    );

    Expect(
        !IsExtendedPictographic(0x1F1EE),
        "Regional indicator not extended pictographic"
    );

    return Summary();
}
