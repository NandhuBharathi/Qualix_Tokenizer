#pragma once

#include <utility>

#include "core/status.hpp"

namespace qualix
{

template<typename T>
class Result
{
public:
    Result(const T& value)
        : value_(value), status_(Status::Success())
    {
    }

    Result(T&& value)
        : value_(std::move(value)), status_(Status::Success())
    {
    }

    Result(Status status)
        : value_{}, status_(status)
    {
    }

    [[nodiscard]]
    bool Ok() const noexcept
    {
        return status_.Ok();
    }

    [[nodiscard]]
    bool Failed() const noexcept
    {
        return status_.Failed();
    }

    [[nodiscard]]
    const Status& GetStatus() const noexcept
    {
        return status_;
    }

    [[nodiscard]]
    const T& Value() const noexcept
    {
        return value_;
    }

    [[nodiscard]]
    T& Value() noexcept
    {
        return value_;
    }

private:
    T value_{};
    class Status status_;
};

} // namespace qualix
