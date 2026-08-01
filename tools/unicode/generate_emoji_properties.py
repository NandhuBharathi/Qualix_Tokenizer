from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

INPUT = ROOT / "tools/unicode/data/emoji-data.txt"
OUTPUT = ROOT / "include/unicode/generated/emoji_property_tables.hpp"

PROPERTIES = {
    "Emoji": "EmojiRanges",
    "Emoji_Presentation": "EmojiPresentationRanges",
    "Emoji_Modifier": "EmojiModifierRanges",
    "Emoji_Modifier_Base": "EmojiModifierBaseRanges",
    "Extended_Pictographic": "ExtendedPictographicRanges",
}

def parse_codepoint_range(value):
    value = value.strip()

    if ".." in value:
        first, last = value.split("..", 1)
        return int(first, 16), int(last, 16)

    cp = int(value, 16)
    return cp, cp

def parse():
    result = {
        prop: []
        for prop in PROPERTIES
    }

    with INPUT.open(
        "r",
        encoding="utf-8"
    ) as f:
        for line_number, raw in enumerate(f, 1):
            line = raw.split("#", 1)[0].strip()

            if not line:
                continue

            if ";" not in line:
                continue

            codepoints, prop = [
                part.strip()
                for part in line.split(";", 1)
            ]

            if prop not in result:
                continue

            first, last = parse_codepoint_range(
                codepoints
            )

            result[prop].append(
                (first, last)
            )

    return result

def merge_ranges(ranges):
    if not ranges:
        return []

    ranges = sorted(ranges)

    merged = [
        list(ranges[0])
    ]

    for first, last in ranges[1:]:
        previous = merged[-1]

        if first <= previous[1] + 1:
            previous[1] = max(
                previous[1],
                last
            )
        else:
            merged.append(
                [first, last]
            )

    return [
        tuple(value)
        for value in merged
    ]

def write_table(
    out,
    cpp_name,
    ranges
):
    out.write(
        f"inline constexpr "
        f"std::array<CodePointRange, "
        f"{len(ranges)}> "
        f"{cpp_name} = {{{{\n"
    )

    for first, last in ranges:
        out.write(
            "    {"
            f"0x{first:04X}, "
            f"0x{last:04X}"
            "},\n"
        )

    out.write("}};\n\n")

def main():
    parsed = parse()

    OUTPUT.parent.mkdir(
        parents=True,
        exist_ok=True
    )

    merged = {}

    for prop, ranges in parsed.items():
        merged[prop] = merge_ranges(ranges)

    with OUTPUT.open(
        "w",
        encoding="utf-8",
        newline="\n"
    ) as out:
        out.write(
            "#pragma once\n\n"
            "#include <array>\n\n"
            '#include "unicode/codepoint.hpp"\n\n'
            "namespace qualix::unicode::generated\n"
            "{\n\n"
            "struct CodePointRange\n"
            "{\n"
            "    CodePoint first;\n"
            "    CodePoint last;\n"
            "};\n\n"
        )

        for prop, cpp_name in PROPERTIES.items():
            write_table(
                out,
                cpp_name,
                merged[prop]
            )

        out.write(
            "} // namespace "
            "qualix::unicode::generated\n"
        )

    print("Input     :", INPUT)
    print("Generated :", OUTPUT)
    print()

    for prop in PROPERTIES:
        print(
            f"{prop:<24}: "
            f"{len(parsed[prop]):>5} source ranges"
            f" -> "
            f"{len(merged[prop]):>5} merged ranges"
        )

if __name__ == "__main__":
    main()
