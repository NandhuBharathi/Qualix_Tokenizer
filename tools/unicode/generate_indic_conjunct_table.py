from pathlib import Path

ROOT = Path("/kaggle/working/Qualix_Tokenizer")
INPUT = ROOT / "tools/unicode/data/DerivedCoreProperties.txt"
OUTPUT = ROOT / "include/unicode/generated/indic_conjunct_table.hpp"

UNICODE_VERSION = "17.0.0"

PROPERTY_MAP = {
    "Consonant": "Consonant",
    "Extend": "Extend",
    "Linker": "Linker",
}

def parse_range(text):
    text = text.strip()

    if ".." in text:
        first, last = text.split("..", 1)
        return int(first, 16), int(last, 16)

    value = int(text, 16)
    return value, value

def parse():
    ranges = []

    with INPUT.open("r", encoding="utf-8") as file:
        for raw_line in file:
            data = raw_line.split("#", 1)[0].strip()

            if not data:
                continue

            parts = [part.strip() for part in data.split(";")]

            if len(parts) < 3:
                continue

            code_range = parts[0]
            property_name = parts[1]
            property_value = parts[2]

            if property_name != "InCB":
                continue

            if property_value not in PROPERTY_MAP:
                continue

            first, last = parse_range(code_range)

            ranges.append(
                (
                    first,
                    last,
                    PROPERTY_MAP[property_value]
                )
            )

    ranges.sort(key=lambda item: item[0])
    return ranges

def merge(ranges):
    if not ranges:
        return []

    result = [ranges[0]]

    for first, last, prop in ranges[1:]:
        prev_first, prev_last, prev_prop = result[-1]

        if prop == prev_prop and first == prev_last + 1:
            result[-1] = (
                prev_first,
                last,
                prop
            )
        else:
            result.append(
                (
                    first,
                    last,
                    prop
                )
            )

    return result

def cp(value):
    return f"0x{value:06X}"

def generate():
    ranges = merge(parse())

    OUTPUT.parent.mkdir(
        parents=True,
        exist_ok=True
    )

    with OUTPUT.open("w", encoding="utf-8") as out:
        out.write("#pragma once\n\n")
        out.write("#include <array>\n\n")
        out.write('#include "unicode/codepoint.hpp"\n')
        out.write('#include "unicode/indic_conjunct.hpp"\n\n')

        out.write("namespace qualix::unicode::generated\n")
        out.write("{\n\n")

        out.write("struct IndicConjunctRange\n")
        out.write("{\n")
        out.write("    CodePoint first;\n")
        out.write("    CodePoint last;\n")
        out.write("    IndicConjunctBreak property;\n")
        out.write("};\n\n")

        out.write(
            f"inline constexpr std::array<IndicConjunctRange, "
            f"{len(ranges)}> IndicConjunctRanges = {{{{\n"
        )

        for first, last, prop in ranges:
            out.write(
                f"    {{{cp(first)}, {cp(last)}, "
                f"IndicConjunctBreak::{prop}}},\n"
            )

        out.write("}};\n\n")
        out.write("} // namespace qualix::unicode::generated\n")

    print(f"Unicode Version          : {UNICODE_VERSION}")
    print(f"Indic Conjunct Ranges    : {len(ranges)}")
    print(f"Generated                : {OUTPUT}")

if __name__ == "__main__":
    generate()
