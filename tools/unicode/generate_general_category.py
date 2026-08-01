from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

INPUT = ROOT / "tools/unicode/data/UnicodeData.txt"
OUTPUT = ROOT / "include/unicode/generated/general_category_tables.hpp"

def parse():
    records = []
    pending = None

    with INPUT.open(
        "r",
        encoding="utf-8"
    ) as file:
        for line_number, raw in enumerate(file, 1):
            raw = raw.rstrip("\n")

            if not raw:
                continue

            fields = raw.split(";")

            if len(fields) < 3:
                raise RuntimeError(
                    f"Invalid UnicodeData record at line {line_number}"
                )

            cp = int(fields[0], 16)
            name = fields[1]
            category = fields[2]

            if name.endswith(", First>"):
                if pending is not None:
                    raise RuntimeError(
                        f"Nested First range at line {line_number}"
                    )

                pending = (
                    cp,
                    category,
                    name
                )
                continue

            if name.endswith(", Last>"):
                if pending is None:
                    raise RuntimeError(
                        f"Last without First at line {line_number}"
                    )

                first, first_category, first_name = pending

                if first_category != category:
                    raise RuntimeError(
                        f"Range category mismatch at line {line_number}"
                    )

                if cp < first:
                    raise RuntimeError(
                        f"Invalid Unicode range at line {line_number}"
                    )

                records.append(
                    [first, cp, category]
                )

                pending = None
                continue

            if pending is not None:
                raise RuntimeError(
                    f"Missing Last range before line {line_number}"
                )

            records.append(
                [cp, cp, category]
            )

    if pending is not None:
        raise RuntimeError(
            "Unclosed UnicodeData First range"
        )

    return records

def merge(records):
    merged = []

    for first, last, category in records:
        if (
            merged and
            merged[-1][2] == category and
            merged[-1][1] + 1 == first
        ):
            merged[-1][1] = last
        else:
            merged.append(
                [first, last, category]
            )

    return merged

def generate(ranges):
    OUTPUT.parent.mkdir(
        parents=True,
        exist_ok=True
    )

    with OUTPUT.open(
        "w",
        encoding="utf-8",
        newline="\n"
    ) as out:
        out.write("#pragma once\n\n")
        out.write("#include <array>\n\n")
        out.write('#include "unicode/codepoint.hpp"\n')
        out.write('#include "unicode/general_category.hpp"\n\n')

        out.write(
            "namespace qualix::unicode::generated\n"
            "{\n\n"
        )

        out.write(
            "struct GeneralCategoryRange\n"
            "{\n"
            "    CodePoint first;\n"
            "    CodePoint last;\n"
            "    GeneralCategory category;\n"
            "};\n\n"
        )

        out.write(
            f"inline constexpr std::array<GeneralCategoryRange, "
            f"{len(ranges)}> GeneralCategoryRanges = {{{{\n"
        )

        for first, last, category in ranges:
            out.write(
                "    {"
                f"0x{first:04X}, "
                f"0x{last:04X}, "
                f"GeneralCategory::{category}"
                "},\n"
            )

        out.write("}};\n\n")
        out.write(
            "} // namespace qualix::unicode::generated\n"
        )

def main():
    records = parse()
    ranges = merge(records)

    generate(ranges)

    print("UnicodeData records :", len(records))
    print("Compressed ranges   :", len(ranges))
    print("Generated           :", OUTPUT)

if __name__ == "__main__":
    main()
