#pragma once

#include "core/types.hpp"

#include <string_view>

namespace qualix::rules
{


struct NumericScanPolicy
{
    bool allow_based_numbers{true};
    bool allow_grouping{true};
    bool allow_underscore{true};
    bool allow_leading_dot{true};
    bool allow_trailing_dot{true};
    bool allow_sign{true};
    bool allow_exponent{true};
};

struct NumericScan
{
    usize start{0};
    usize end{0};
    bool matched{false};

    [[nodiscard]]
    usize ByteLength() const noexcept
    {
        return end-start;
    }
};

class NumericScanner
{
public:
    [[nodiscard]]
    static NumericScan Scan(
        std::string_view input,
        usize offset
    ) noexcept;

    [[nodiscard]]
    static NumericScan ScanRaw(
        std::string_view input,
        usize offset
    ) noexcept;

    [[nodiscard]]
    static NumericScan ScanRaw(
        std::string_view input,
        usize offset,
        const NumericScanPolicy& policy
    ) noexcept;

};

} // namespace qualix::rules
