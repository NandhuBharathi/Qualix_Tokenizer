#pragma once

#include <string_view>

#include "core/types.hpp"

namespace qualix::unicode
{

struct Grapheme
{
    usize byte_start = 0;
    usize byte_length = 0;

    [[nodiscard]]
    constexpr usize ByteEnd() const noexcept
    {
        return byte_start + byte_length;
    }

    [[nodiscard]]
    constexpr bool Empty() const noexcept
    {
        return byte_length == 0;
    }

    [[nodiscard]]
    std::string_view View(std::string_view source) const noexcept
    {
        if (byte_start > source.size())
            return {};

        if (byte_length > source.size() - byte_start)
            return {};

        return source.substr(byte_start, byte_length);
    }
};

} // namespace qualix::unicode
