from pathlib import Path

ROOT = Path("/kaggle/working/Qualix_Tokenizer")

UNICODE_DATA = ROOT / "tools/unicode/data/UnicodeData.txt"
DERIVED_PROPS = ROOT / "tools/unicode/data/DerivedNormalizationProps.txt"

OUTPUT = (
    ROOT /
    "include/unicode/generated/normalization_tables.hpp"
)

ccc_entries = []
decomp_entries = []
decomp_data = []
canonical_mappings = {}
compat_decomp_entries = []
compat_decomp_data = []

composition_exclusions = set()

with DERIVED_PROPS.open("r", encoding="utf-8") as f:
    for raw in f:
        line = raw.split("#", 1)[0].strip()

        if not line or ";" not in line:
            continue

        left, prop = [
            x.strip()
            for x in line.split(";", 1)
        ]

        if prop != "Full_Composition_Exclusion":
            continue

        if ".." in left:
            first, last = left.split("..")

            first = int(first, 16)
            last = int(last, 16)

            composition_exclusions.update(
                range(first, last + 1)
            )
        else:
            composition_exclusions.add(
                int(left, 16)
            )

with UNICODE_DATA.open("r", encoding="utf-8") as f:
    for line in f:
        fields = line.rstrip("\n").split(";")

        if len(fields) < 6:
            continue

        cp = int(fields[0], 16)
        ccc = int(fields[3])
        decomposition = fields[5].strip()

        if ccc != 0:
            ccc_entries.append((cp, ccc))

        if decomposition:
            parts = decomposition.split()

            is_compatibility = parts[0].startswith("<")

            if is_compatibility:
                parts = parts[1:]

            mapping = [
                int(x, 16)
                for x in parts
            ]

            compat_offset = len(compat_decomp_data)
            compat_decomp_data.extend(mapping)

            compat_decomp_entries.append(
                (cp, compat_offset, len(mapping))
            )

            if not is_compatibility:
                offset = len(decomp_data)
                decomp_data.extend(mapping)

                decomp_entries.append(
                    (cp, offset, len(mapping))
                )

                canonical_mappings[cp] = mapping

composition_entries = []

for composed, mapping in canonical_mappings.items():
    if composed in composition_exclusions:
        continue

    if len(mapping) != 2:
        continue

    first, second = mapping

    composition_entries.append(
        (first, second, composed)
    )

composition_entries.sort(
    key=lambda x: (x[0], x[1])
)

OUTPUT.parent.mkdir(
    parents=True,
    exist_ok=True
)

with OUTPUT.open("w", encoding="utf-8") as out:
    out.write(
'''#pragma once

#include <array>

#include "unicode/codepoint.hpp"

namespace qualix::unicode::generated
{

inline constexpr const char*
NormalizationUnicodeVersion = "17.0.0";

struct CombiningClassEntry
{
    CodePoint codepoint;
    unsigned char value;
};

struct CanonicalDecompositionEntry
{
    CodePoint codepoint;
    unsigned int offset;
    unsigned char length;
};

struct CanonicalCompositionEntry
{
    CodePoint first;
    CodePoint second;
    CodePoint composed;
};

struct CompatibilityDecompositionEntry
{
    CodePoint codepoint;
    unsigned int offset;
    unsigned char length;
};

'''
    )

    out.write(
        f"inline constexpr std::array"
        f"<CombiningClassEntry, "
        f"{len(ccc_entries)}> "
        f"CombiningClassTable = {{{{\n"
    )

    for cp, ccc in ccc_entries:
        out.write(
            f"    {{0x{cp:04X}, {ccc}}},\n"
        )

    out.write("}};\n\n")

    out.write(
        f"inline constexpr std::array"
        f"<CodePoint, {len(decomp_data)}> "
        f"CanonicalDecompositionData = {{{{\n"
    )

    for cp in decomp_data:
        out.write(
            f"    0x{cp:04X},\n"
        )

    out.write("}};\n\n")

    out.write(
        f"inline constexpr std::array"
        f"<CanonicalDecompositionEntry, "
        f"{len(decomp_entries)}> "
        f"CanonicalDecompositionTable = {{{{\n"
    )

    for cp, offset, length in decomp_entries:
        out.write(
            f"    {{0x{cp:04X}, "
            f"{offset}, {length}}},\n"
        )

    out.write("}};\n\n")

    out.write(
        f"inline constexpr std::array"
        f"<CanonicalCompositionEntry, "
        f"{len(composition_entries)}> "
        f"CanonicalCompositionTable = {{{{\n"
    )

    for first, second, composed in composition_entries:
        out.write(
            f"    {{0x{first:04X}, "
            f"0x{second:04X}, "
            f"0x{composed:04X}}},\n"
        )

    out.write("}};\n\n")

    out.write(
        f"inline constexpr std::array"
        f"<CodePoint, {len(compat_decomp_data)}> "
        f"CompatibilityDecompositionData = {{{{\n"
    )

    for cp in compat_decomp_data:
        out.write(
            f"    0x{cp:04X},\n"
        )

    out.write("}};\n\n")

    out.write(
        f"inline constexpr std::array"
        f"<CompatibilityDecompositionEntry, "
        f"{len(compat_decomp_entries)}> "
        f"CompatibilityDecompositionTable = {{{{\n"
    )

    for cp, offset, length in compat_decomp_entries:
        out.write(
            f"    {{0x{cp:04X}, "
            f"{offset}, {length}}},\n"
        )

    out.write(
        "}};\n\n"
        "} // namespace qualix::unicode::generated\n"
    )

print("Unicode Version           : 17.0.0")
print("Combining Class Entries   :", len(ccc_entries))
print("Canonical Decompositions  :", len(decomp_entries))
print("Composition Exclusions    :", len(composition_exclusions))
print("Canonical Compositions    :", len(composition_entries))
print("All Decompositions        :", len(compat_decomp_entries))
print("Generated                 :", OUTPUT)
