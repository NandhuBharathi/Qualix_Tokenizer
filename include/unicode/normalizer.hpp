#pragma once

#include <string>
#include <string_view>

#include "core/result.hpp"

namespace qualix::unicode
{

enum class NormalizationForm
{
    None,
    NFC,
    NFD,
    NFKC,
    NFKD
};

class Normalizer
{
public:
    [[nodiscard]]
    static Result<std::string> Normalize(
        std::string_view input,
        NormalizationForm form
    );

    [[nodiscard]]
    static bool IsNormalized(
        std::string_view input,
        NormalizationForm form
    );
};

} // namespace qualix::unicode
