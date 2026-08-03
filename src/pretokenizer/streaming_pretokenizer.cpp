#include "pretokenizer/streaming_pretokenizer.hpp"

#include <algorithm>
#include <utility>

#include "pretokenizer/pretokenizer.hpp"

namespace qualix::pretokenizer
{

namespace
{

bool IsAsciiSpace(char c) noexcept
{
    return
        c==' '||
        c=='\t'||
        c=='\n'||
        c=='\r';
}

/*
 * Return the beginning of the final raw field.
 *
 * Chunk boundaries have no meaning here.
 *
 * Example:
 *
 * pending = "Hello wor"
 *
 * safe  = "Hello "
 * carry = "wor"
 *
 * Next:
 *
 * carry + "ld test"
 * = "world test"
 *
 * safe  = "world "
 * carry = "test"
 */
usize FindLastFieldStart(
    std::string_view input
) noexcept
{
    if(input.empty())
        return 0;

    usize p=input.size();

    /*
     * If input ends in actual whitespace, there is
     * no partial field after that whitespace.
     *
     * Everything is safe.
     */
    if(IsAsciiSpace(input[p-1]))
        return p;

    /*
     * Walk backwards through the final field.
     */
    while(p>0&&!IsAsciiSpace(input[p-1]))
        --p;

    return p;
}

usize GraphemeExtent(
    const std::vector<Span>& spans
) noexcept
{
    usize extent=0;

    for(const auto& span:spans)
    {
        extent=std::max(
            extent,
            span.GraphemeEnd()
        );
    }

    return extent;
}

} // namespace

void StreamingPreTokenizer::Feed(
    std::string_view chunk
)
{
    if(finished_||chunk.empty())
        return;

    /*
     * Previous carry + current raw chunk.
     *
     * A chunk boundary is NOT inserted into
     * pending_ and therefore has no lexical
     * or semantic meaning.
     */
    pending_.append(
        chunk.data(),
        chunk.size()
    );

    peak_buffered_bytes_=std::max(
        peak_buffered_bytes_,
        pending_.size()
    );

    ProcessSafePrefix();
}

void StreamingPreTokenizer::ProcessSafePrefix()
{
    if(pending_.empty())
        return;

    const usize safe_bytes=
        FindLastFieldStart(
            pending_
        );

    /*
     * Entire pending buffer is currently one
     * unfinished field.
     */
    if(safe_bytes==0)
        return;

    const std::string_view safe(
        pending_.data(),
        safe_bytes
    );

    auto result=
        PreTokenizer::Split(
            safe
        );

    /*
     * Feed() cannot propagate Result errors.
     * Preserve pending bytes and allow Finish()
     * to perform the authoritative parse.
     */
    if(result.Failed())
        return;

    auto spans=
        std::move(
            result.Value()
        );

    const usize graphemes=
        GraphemeExtent(
            spans
        );

    for(auto& span:spans)
    {
        span.byte_start+=
            processed_bytes_;

        span.grapheme_start+=
            processed_graphemes_;

        completed_.push_back(
            std::move(span)
        );
    }

    processed_bytes_+=
        safe_bytes;

    processed_graphemes_+=
        graphemes;

    pending_.erase(
        0,
        safe_bytes
    );
}

Result<std::vector<Span>>
StreamingPreTokenizer::Finish()
{
    if(finished_)
        return completed_;

    finished_=true;

    if(pending_.empty())
        return completed_;

    /*
     * Final carry has no future chunk.
     * Process all remaining bytes.
     */
    auto result=
        PreTokenizer::Split(
            pending_
        );

    if(result.Failed())
        return result.GetStatus();

    auto spans=
        std::move(
            result.Value()
        );

    const usize graphemes=
        GraphemeExtent(
            spans
        );

    for(auto& span:spans)
    {
        span.byte_start+=
            processed_bytes_;

        span.grapheme_start+=
            processed_graphemes_;

        completed_.push_back(
            std::move(span)
        );
    }

    processed_bytes_+=
        pending_.size();

    processed_graphemes_+=
        graphemes;

    pending_.clear();

    return completed_;
}

void StreamingPreTokenizer::Reset() noexcept
{
    pending_.clear();
    completed_.clear();

    processed_bytes_=0;
    processed_graphemes_=0;
    peak_buffered_bytes_=0;

    finished_=false;
}

} // namespace qualix::pretokenizer
