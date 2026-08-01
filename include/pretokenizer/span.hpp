#pragma once

#include <string_view>

#include "core/types.hpp"
#include "pretokenizer/span_type.hpp"

namespace qualix::pretokenizer
{

enum class SpanPolicy
{
    Splittable = 0,
    Protected
};

struct Span
{
    usize byte_start = 0;
    usize byte_length = 0;

    usize grapheme_start = 0;
    usize grapheme_count = 0;

    SpanType type = SpanType::Unknown;
    SpanPolicy policy = SpanPolicy::Splittable;

    constexpr usize ByteEnd() const noexcept
    {
        return byte_start + byte_length;
    }

    constexpr usize GraphemeEnd() const noexcept
    {
        return grapheme_start + grapheme_count;
    }

    constexpr bool Empty() const noexcept
    {
        return byte_length == 0;
    }

    constexpr bool Protected() const noexcept
    {
        return policy == SpanPolicy::Protected;
    }

    std::string_view View(
        std::string_view input
    ) const noexcept
    {
        if (byte_start > input.size())
            return {};

        if (byte_length >
            input.size() - byte_start)
        {
            return {};
        }

        return input.substr(
            byte_start,
            byte_length
        );
    }
};

} // namespace qualix::pretokenizer
