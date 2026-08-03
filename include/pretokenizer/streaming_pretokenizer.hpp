#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "core/result.hpp"
#include "pretokenizer/span.hpp"

namespace qualix::pretokenizer
{

class StreamingPreTokenizer
{
public:
    StreamingPreTokenizer() = default;

    void Feed(
        std::string_view chunk
    );

    [[nodiscard]]
    Result<std::vector<Span>> Finish();

    [[nodiscard]]
    usize BufferedBytes() const noexcept
    {
        return pending_.size();
    }

    [[nodiscard]]
    usize PeakBufferedBytes() const noexcept
    {
        return peak_buffered_bytes_;
    }

    [[nodiscard]]
    usize ProcessedBytes() const noexcept
    {
        return processed_bytes_;
    }

    [[nodiscard]]
    usize EmittedSpans() const noexcept
    {
        return completed_.size();
    }

    [[nodiscard]]
    bool Finished() const noexcept
    {
        return finished_;
    }

    void Reset() noexcept;

private:
    void ProcessSafePrefix();

    std::string pending_;
    std::vector<Span> completed_;

    usize processed_bytes_ = 0;
    usize processed_graphemes_ = 0;
    usize peak_buffered_bytes_ = 0;

    bool finished_ = false;
};

} // namespace qualix::pretokenizer
