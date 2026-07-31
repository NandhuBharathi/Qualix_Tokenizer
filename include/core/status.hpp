#pragma once

#include "core/error.hpp"

namespace qualix
{

class Status
{
public:
    constexpr Status() noexcept
        : error_(ErrorCode::None)
    {
    }

    constexpr explicit Status(ErrorCode error) noexcept
        : error_(error)
    {
    }

    [[nodiscard]]
    constexpr bool Ok() const noexcept
    {
        return error_ == ErrorCode::None;
    }

    [[nodiscard]]
    constexpr bool Failed() const noexcept
    {
        return error_ != ErrorCode::None;
    }

    [[nodiscard]]
    constexpr ErrorCode Code() const noexcept
    {
        return error_;
    }

    [[nodiscard]]
    constexpr const char* Message() const noexcept
    {
        return ToString(error_).data();
    }

    [[nodiscard]]
    static constexpr Status Success() noexcept
    {
        return Status{};
    }

    [[nodiscard]]
    static constexpr Status Failure(ErrorCode error) noexcept
    {
        return Status(error);
    }

private:
    ErrorCode error_;
};

} // namespace qualix
