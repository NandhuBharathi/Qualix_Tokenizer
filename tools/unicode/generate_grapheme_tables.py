from pathlib import Path

ROOT = Path("/kaggle/working/Qualix_Tokenizer")
DATA = ROOT / "tools/unicode/data"
OUTPUT = ROOT / "include/unicode/generated/grapheme_tables.hpp"

UNICODE_VERSION = "17.0.0"

GCB_PROPERTIES = {
    "CR",
    "LF",
    "Control",
    "Extend",
    "ZWJ",
    "Regional_Indicator",
    "Prepend",
    "SpacingMark",
    "L",
    "V",
    "T",
    "LV",
    "LVT",
}

PROPERTY_MAP = {
    "CR": "CR",
    "LF": "LF",
    "Control": "Control",
    "Extend": "Extend",
    "ZWJ": "ZWJ",
    "Regional_Indicator": "RegionalIndicator",
    "Prepend": "Prepend",
    "SpacingMark": "SpacingMark",
    "L": "L",
    "V": "V",
    "T": "T",
    "LV": "LV",
    "LVT": "LVT",
}

def parse_range(text):
    text = text.strip()
    if ".." in text:
        start, end = text.split("..")
        return int(start, 16), int(end, 16)
    value = int(text, 16)
    return value, value

def parse_property_file(path):
    ranges = []

    with path.open("r", encoding="utf-8") as file:
        for raw_line in file:
            line = raw_line.split("#", 1)[0].strip()

            if not line or ";" not in line:
                continue

            code_range, property_name = [
                part.strip() for part in line.split(";", 1)
            ]

            if property_name not in GCB_PROPERTIES:
                continue

            start, end = parse_range(code_range)

            ranges.append(
                (
                    start,
                    end,
                    PROPERTY_MAP[property_name],
                )
            )

    ranges.sort(key=lambda item: item[0])
    return ranges

def parse_extended_pictographic(path):
    ranges = []

    with path.open("r", encoding="utf-8") as file:
        for raw_line in file:
            line = raw_line.split("#", 1)[0].strip()

            if not line or ";" not in line:
                continue

            code_range, property_name = [
                part.strip() for part in line.split(";", 1)
            ]

            if property_name != "Extended_Pictographic":
                continue

            ranges.append(parse_range(code_range))

    ranges.sort(key=lambda item: item[0])
    return ranges

def merge_property_ranges(ranges):
    if not ranges:
        return []

    merged = [ranges[0]]

    for start, end, prop in ranges[1:]:
        last_start, last_end, last_prop = merged[-1]

        if prop == last_prop and start == last_end + 1:
            merged[-1] = (
                last_start,
                end,
                prop,
            )
        else:
            merged.append((start, end, prop))

    return merged

def merge_ranges(ranges):
    if not ranges:
        return []

    merged = [ranges[0]]

    for start, end in ranges[1:]:
        last_start, last_end = merged[-1]

        if start <= last_end + 1:
            merged[-1] = (
                last_start,
                max(last_end, end),
            )
        else:
            merged.append((start, end))

    return merged

def hex_cp(value):
    return f"0x{value:06X}"

def generate():
    gcb = parse_property_file(
        DATA / "GraphemeBreakProperty.txt"
    )

    pictographic = parse_extended_pictographic(
        DATA / "emoji-data.txt"
    )

    gcb = merge_property_ranges(gcb)
    pictographic = merge_ranges(pictographic)

    OUTPUT.parent.mkdir(parents=True, exist_ok=True)

    with OUTPUT.open("w", encoding="utf-8") as out:
        out.write("#pragma once\n\n")
        out.write("#include <array>\n")
        out.write("#include <string_view>\n\n")
        out.write('#include "unicode/codepoint.hpp"\n')
        out.write('#include "unicode/grapheme_property.hpp"\n\n')

        out.write("namespace qualix::unicode::generated\n")
        out.write("{\n\n")

        out.write(
            f'inline constexpr std::string_view UnicodeVersion = '
            f'"{UNICODE_VERSION}";\n\n'
        )

        out.write("struct GraphemePropertyRange\n")
        out.write("{\n")
        out.write("    CodePoint first;\n")
        out.write("    CodePoint last;\n")
        out.write("    GraphemeBreakProperty property;\n")
        out.write("};\n\n")

        out.write("struct CodePointRange\n")
        out.write("{\n")
        out.write("    CodePoint first;\n")
        out.write("    CodePoint last;\n")
        out.write("};\n\n")

        out.write(
            f"inline constexpr std::array<GraphemePropertyRange, "
            f"{len(gcb)}> GraphemePropertyRanges = {{{{\n"
        )

        for start, end, prop in gcb:
            out.write(
                f"    {{{hex_cp(start)}, {hex_cp(end)}, "
                f"GraphemeBreakProperty::{prop}}},\n"
            )

        out.write("}};\n\n")

        out.write(
            f"inline constexpr std::array<CodePointRange, "
            f"{len(pictographic)}> ExtendedPictographicRanges = {{{{\n"
        )

        for start, end in pictographic:
            out.write(
                f"    {{{hex_cp(start)}, {hex_cp(end)}}},\n"
            )

        out.write("}};\n\n")
        out.write("} // namespace qualix::unicode::generated\n")

    print(f"Unicode Version              : {UNICODE_VERSION}")
    print(f"Grapheme Property Ranges     : {len(gcb)}")
    print(f"Extended Pictographic Ranges : {len(pictographic)}")
    print(f"Generated                    : {OUTPUT}")

if __name__ == "__main__":
    generate()
