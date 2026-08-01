#include "unicode/normalizer.hpp"

#include <algorithm>
#include <vector>
#include <utility>

#include "core/error.hpp"
#include "unicode/codepoint.hpp"
#include "unicode/generated/normalization_tables.hpp"
#include "unicode/utf8.hpp"

namespace qualix::unicode
{

namespace
{

using generated::CanonicalDecompositionData;
using generated::CanonicalDecompositionTable;
using generated::CombiningClassTable;

constexpr CodePoint SBase = 0xAC00;
constexpr CodePoint LBase = 0x1100;
constexpr CodePoint VBase = 0x1161;
constexpr CodePoint TBase = 0x11A7;

constexpr int LCount = 19;
constexpr int VCount = 21;
constexpr int TCount = 28;
constexpr int NCount = VCount * TCount;
constexpr int SCount = LCount * NCount;

unsigned char CombiningClass(CodePoint cp) noexcept
{
    auto it = std::lower_bound(
        CombiningClassTable.begin(),
        CombiningClassTable.end(),
        cp,
        [](const auto& entry, CodePoint value)
        {
            return entry.codepoint < value;
        }
    );

    if (it != CombiningClassTable.end() &&
        it->codepoint == cp)
    {
        return it->value;
    }

    return 0;
}

const generated::CanonicalDecompositionEntry*
FindCanonicalDecomposition(CodePoint cp) noexcept
{
    auto it = std::lower_bound(
        CanonicalDecompositionTable.begin(),
        CanonicalDecompositionTable.end(),
        cp,
        [](const auto& entry, CodePoint value)
        {
            return entry.codepoint < value;
        }
    );

    if (it == CanonicalDecompositionTable.end() ||
        it->codepoint != cp)
    {
        return nullptr;
    }

    return &*it;
}

bool IsHangulSyllable(CodePoint cp) noexcept
{
    return cp >= SBase &&
           cp < SBase + static_cast<CodePoint>(SCount);
}

void Decompose(
    CodePoint cp,
    std::vector<CodePoint>& output
)
{
    if (IsHangulSyllable(cp))
    {
        const int s_index =
            static_cast<int>(cp - SBase);

        const CodePoint l =
            LBase + static_cast<CodePoint>(
                s_index / NCount
            );

        const CodePoint v =
            VBase + static_cast<CodePoint>(
                (s_index % NCount) / TCount
            );

        const CodePoint t =
            TBase + static_cast<CodePoint>(
                s_index % TCount
            );

        output.push_back(l);
        output.push_back(v);

        if (t != TBase)
            output.push_back(t);

        return;
    }

    const auto* mapping =
        FindCanonicalDecomposition(cp);

    if (!mapping)
    {
        output.push_back(cp);
        return;
    }

    for (unsigned int i = 0; i < mapping->length; ++i)
    {
        const CodePoint child =
            CanonicalDecompositionData[
                mapping->offset + i
            ];

        // Canonical decomposition is recursive.
        Decompose(child, output);
    }
}

void CanonicalReorder(
    std::vector<CodePoint>& codepoints
)
{
    // Stable insertion ordering by Canonical Combining Class.
    // A CCC=0 code point starts a new canonical sequence.
    for (usize i = 1; i < codepoints.size(); ++i)
    {
        const auto current_ccc =
            CombiningClass(codepoints[i]);

        if (current_ccc == 0)
            continue;

        usize j = i;

        while (j > 0)
        {
            const auto previous_ccc =
                CombiningClass(codepoints[j - 1]);

            if (previous_ccc == 0 ||
                previous_ccc <= current_ccc)
            {
                break;
            }

            std::swap(
                codepoints[j],
                codepoints[j - 1]
            );

            --j;
        }
    }
}

Result<std::vector<CodePoint>> DecodeAll(
    std::string_view input
)
{
    std::vector<CodePoint> codepoints;
    codepoints.reserve(input.size());

    usize offset = 0;

    while (offset < input.size())
    {
        auto decoded =
            Utf8::Decode(input.substr(offset));

        if (decoded.Failed())
            return Status::Failure(
                ErrorCode::InvalidUtf8
            );

        const auto value = decoded.Value();

        codepoints.push_back(value.codepoint);
        offset += value.bytes_consumed;
    }

    return codepoints;
}

Result<std::string> EncodeAll(
    const std::vector<CodePoint>& codepoints
)
{
    std::string output;

    for (const CodePoint cp : codepoints)
    {
        auto encoded = Utf8::Encode(cp);

        if (encoded.Failed())
            return Status::Failure(
                ErrorCode::InvalidCodePoint
            );

        output += encoded.Value();
    }

    return output;
}

Result<std::string> NormalizeNfd(
    std::string_view input
)
{
    auto decoded = DecodeAll(input);

    if (decoded.Failed())
        return Status::Failure(
            ErrorCode::InvalidUtf8
        );

    std::vector<CodePoint> decomposed;

    for (const CodePoint cp : decoded.Value())
        Decompose(cp, decomposed);

    CanonicalReorder(decomposed);

    return EncodeAll(decomposed);
}


bool ComposeHangul(
    CodePoint first,
    CodePoint second,
    CodePoint& composed
) noexcept
{
    if (first >= LBase &&
        first < LBase + LCount &&
        second >= VBase &&
        second < VBase + VCount)
    {
        const auto l_index =
            static_cast<int>(first - LBase);

        const auto v_index =
            static_cast<int>(second - VBase);

        composed =
            SBase +
            static_cast<CodePoint>(
                (l_index * VCount + v_index) *
                TCount
            );

        return true;
    }

    if (first >= SBase &&
        first < SBase + SCount &&
        ((first - SBase) % TCount) == 0 &&
        second > TBase &&
        second < TBase + TCount)
    {
        composed =
            first +
            (second - TBase);

        return true;
    }

    return false;
}

bool ComposePair(
    CodePoint first,
    CodePoint second,
    CodePoint& composed
) noexcept
{
    if (ComposeHangul(first, second, composed))
        return true;

    using generated::CanonicalCompositionTable;

    auto it = std::lower_bound(
        CanonicalCompositionTable.begin(),
        CanonicalCompositionTable.end(),
        std::pair<CodePoint, CodePoint>{
            first,
            second
        },
        [](const auto& entry, const auto& value)
        {
            if (entry.first != value.first)
                return entry.first < value.first;

            return entry.second < value.second;
        }
    );

    if (it == CanonicalCompositionTable.end() ||
        it->first != first ||
        it->second != second)
    {
        return false;
    }

    composed = it->composed;
    return true;
}

void CanonicalCompose(
    std::vector<CodePoint>& codepoints
)
{
    if (codepoints.empty())
        return;

    std::vector<CodePoint> output;
    output.reserve(codepoints.size());

    output.push_back(codepoints[0]);

    usize starter_pos = 0;
    CodePoint starter = output[0];

    unsigned char last_ccc =
        CombiningClass(output[0]);

    for (usize i = 1; i < codepoints.size(); ++i)
    {
        const CodePoint current =
            codepoints[i];

        const unsigned char current_ccc =
            CombiningClass(current);

        CodePoint composite = 0;

        const bool unblocked =
            last_ccc < current_ccc ||
            last_ccc == 0;

        if (unblocked &&
            ComposePair(starter, current, composite))
        {
            output[starter_pos] = composite;
            starter = composite;
            continue;
        }

        if (current_ccc == 0)
        {
            starter_pos = output.size();
            starter = current;
        }

        output.push_back(current);
        last_ccc = current_ccc;
    }

    codepoints.swap(output);
}

Result<std::string> NormalizeNfc(
    std::string_view input
)
{
    auto nfd = NormalizeNfd(input);

    if (nfd.Failed())
        return Status::Failure(
            ErrorCode::InvalidUtf8
        );

    auto decoded = DecodeAll(nfd.Value());

    if (decoded.Failed())
        return Status::Failure(
            ErrorCode::InvalidUtf8
        );

    auto codepoints =
        std::move(decoded.Value());

    CanonicalCompose(codepoints);

    return EncodeAll(codepoints);
}



const generated::CompatibilityDecompositionEntry*
FindCompatibilityDecomposition(
    CodePoint cp
) noexcept
{
    using generated::CompatibilityDecompositionTable;

    auto it = std::lower_bound(
        CompatibilityDecompositionTable.begin(),
        CompatibilityDecompositionTable.end(),
        cp,
        [](const auto& entry, CodePoint value)
        {
            return entry.codepoint < value;
        }
    );

    if (it == CompatibilityDecompositionTable.end() ||
        it->codepoint != cp)
    {
        return nullptr;
    }

    return &*it;
}

void CompatibilityDecompose(
    CodePoint cp,
    std::vector<CodePoint>& output
)
{
    if (IsHangulSyllable(cp))
    {
        Decompose(cp, output);
        return;
    }

    const auto* mapping =
        FindCompatibilityDecomposition(cp);

    if (!mapping)
    {
        output.push_back(cp);
        return;
    }

    using generated::CompatibilityDecompositionData;

    for (unsigned int i = 0;
         i < mapping->length;
         ++i)
    {
        const CodePoint child =
            CompatibilityDecompositionData[
                mapping->offset + i
            ];

        CompatibilityDecompose(
            child,
            output
        );
    }
}

Result<std::string> NormalizeNfkd(
    std::string_view input
)
{
    auto decoded = DecodeAll(input);

    if (decoded.Failed())
        return Status::Failure(
            ErrorCode::InvalidUtf8
        );

    std::vector<CodePoint> decomposed;

    for (const CodePoint cp : decoded.Value())
    {
        CompatibilityDecompose(
            cp,
            decomposed
        );
    }

    CanonicalReorder(decomposed);

    return EncodeAll(decomposed);
}


Result<std::string> NormalizeNfkc(
    std::string_view input
)
{
    auto nfkd = NormalizeNfkd(input);

    if (nfkd.Failed())
        return Status::Failure(
            ErrorCode::InvalidUtf8
        );

    auto decoded = DecodeAll(nfkd.Value());

    if (decoded.Failed())
        return Status::Failure(
            ErrorCode::InvalidUtf8
        );

    auto codepoints =
        std::move(decoded.Value());

    CanonicalCompose(codepoints);

    return EncodeAll(codepoints);
}


} // namespace

Result<std::string> Normalizer::Normalize(
    std::string_view input,
    NormalizationForm form
)
{
    if (!Utf8::Validate(input))
        return Status::Failure(
            ErrorCode::InvalidUtf8
        );

    switch (form)
    {
        case NormalizationForm::None:
            return std::string(input);

        case NormalizationForm::NFD:
            return NormalizeNfd(input);

        case NormalizationForm::NFC:
            return NormalizeNfc(input);

        case NormalizationForm::NFKD:
            return NormalizeNfkd(input);

        case NormalizationForm::NFKC:
            return NormalizeNfkc(input);
    }

    return Status::Failure(
        ErrorCode::InvalidArgument
    );
}

bool Normalizer::IsNormalized(
    std::string_view input,
    NormalizationForm form
)
{
    if (!Utf8::Validate(input))
        return false;

    if (form == NormalizationForm::None)
        return true;

    auto normalized = Normalize(input, form);

    if (normalized.Failed())
        return false;

    return normalized.Value() == input;
}

} // namespace qualix::unicode
