#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "pretokenizer/streaming_pretokenizer.hpp"

using namespace qualix;
using namespace qualix::pretokenizer;

namespace
{

bool RunChunks(
    std::string_view text,
    const std::vector<usize>& sizes,
    usize* peak=nullptr
)
{
    if(sizes.empty())
        return false;

    StreamingPreTokenizer stream;

    usize offset=0;
    usize index=0;

    while(offset<text.size())
    {
        const usize wanted=
            sizes[index%sizes.size()];

        const usize count=
            std::min(
                wanted,
                text.size()-offset
            );

        stream.Feed(
            text.substr(
                offset,
                count
            )
        );

        offset+=count;
        ++index;
    }

    if(peak)
        *peak=stream.PeakBufferedBytes();

    const auto result=
        stream.Finish();

    if(result.Failed())
        return false;

    /*
     * Transport-level invariants only.
     *
     * We deliberately do NOT compare the exact
     * semantic span layout against batch mode.
     *
     * Streaming owns:
     *
     *   - raw chunk stitching
     *   - byte continuity
     *   - bounded carry
     *
     * Router/rules own semantic grouping.
     */

    const auto& spans=result.Value();

    if(text.empty())
        return spans.empty();

    if(spans.empty())
        return false;

    usize expected_byte=0;

    for(const auto& span:spans)
    {
        /*
         * No lost bytes.
         * No duplicated bytes.
         * No reordered bytes.
         */
        if(span.byte_start!=expected_byte)
            return false;

        if(span.byte_length==0)
            return false;

        if(span.byte_start>text.size())
            return false;

        if(span.byte_length>
           text.size()-span.byte_start)
            return false;

        expected_byte+=span.byte_length;
    }

    return expected_byte==text.size();
}

bool TestReset()
{
    StreamingPreTokenizer stream;

    stream.Feed(
        "discard this incomplete wor"
    );

    stream.Reset();

    if(stream.BufferedBytes()!=0)
        return false;

    if(stream.ProcessedBytes()!=0)
        return false;

    if(stream.EmittedSpans()!=0)
        return false;

    if(stream.Finished())
        return false;

    stream.Feed(
        "Hello world"
    );

    const auto result=
        stream.Finish();

    if(result.Failed())
        return false;

    const auto& spans=result.Value();

    if(spans.empty())
        return false;

    usize bytes=0;

    for(const auto& span:spans)
    {
        if(span.byte_start!=bytes)
            return false;

        bytes+=span.byte_length;
    }

    return bytes==11;
}

} // namespace

int main()
{
    /*
     * Clean corpus.
     *
     * Chunk boundaries may occur anywhere:
     *
     *   H|e|l|l|o
     *   U|S|D
     *   ₹1|,25|,000
     *   தமிழ|்
     *
     * Those cuts are transport boundaries,
     * NOT characters inserted into the text.
     */
    const std::string unit=
        "Hello world "
        "Price is USD 500 today "
        "Value EUR 1,000.50 end "
        "Weight is 5 kg today "
        "Money ₹1,25,000 end "
        "Visit https://example.com/path?q=123 "
        "Email test@example.com end "
        "Value 75% "
        "Math x+10=20 "
        "தமிழ் மொழி "
        "😀 end. ";

    std::string corpus;
    corpus.reserve(unit.size()*2000);

    for(int i=0;i<2000;++i)
        corpus+=unit;

    usize peak1=0;
    usize peak64=0;
    usize peak4096=0;
    usize peakIrregular=0;

    const bool byte_chunks=
        RunChunks(
            corpus,
            {1},
            &peak1
        );

    const bool irregular=
        RunChunks(
            corpus,
            {1,3,2,7,5,11,17},
            &peakIrregular
        );

    const bool chunk64=
        RunChunks(
            corpus,
            {64},
            &peak64
        );

    const bool chunk4096=
        RunChunks(
            corpus,
            {4096},
            &peak4096
        );

    /*
     * Explicit word-cut contract.
     *
     * These chunks reconstruct:
     *
     *   "Hello world"
     *
     * No whitespace is inserted at chunk cuts.
     */
    const bool word_cut=
        RunChunks(
            "Hello world",
            {2,1,2,1,3,2}
        );

    /*
     * UTF-8 / punctuation / numeric cuts.
     */
    const bool mixed_cut=
        RunChunks(
            "தமிழ் ₹1,25,000 test@example.com 😀",
            {1,2,3,1,5,2,7}
        );

    const bool reset=
        TestReset();

    /*
     * Bounded carry contract.
     *
     * We don't require a magic fixed number.
     * We require that streaming does not retain
     * the complete corpus.
     */
    const bool bounded=
        peak1<corpus.size()&&
        peakIrregular<corpus.size()&&
        peak64<corpus.size()&&
        peak4096<corpus.size();

    const bool pass=
        byte_chunks&&
        irregular&&
        chunk64&&
        chunk4096&&
        word_cut&&
        mixed_cut&&
        reset&&
        bounded;

    std::cout
        <<"QUALIX — #101 RAW STREAMING CONTRACT\n"
        <<"Corpus bytes : "<<corpus.size()<<"\n"
        <<"1-byte       : "<<(byte_chunks?"PASS":"FAIL")<<"\n"
        <<"Irregular    : "<<(irregular?"PASS":"FAIL")<<"\n"
        <<"64-byte      : "<<(chunk64?"PASS":"FAIL")<<"\n"
        <<"4096-byte    : "<<(chunk4096?"PASS":"FAIL")<<"\n"
        <<"Word cut     : "<<(word_cut?"PASS":"FAIL")<<"\n"
        <<"Mixed UTF-8  : "<<(mixed_cut?"PASS":"FAIL")<<"\n"
        <<"Reset        : "<<(reset?"PASS":"FAIL")<<"\n"
        <<"Bounded      : "<<(bounded?"PASS":"FAIL")<<"\n"
        <<"Peak 1       : "<<peak1<<"\n"
        <<"Peak 64      : "<<peak64<<"\n"
        <<"Peak 4096    : "<<peak4096<<"\n"
        <<"Result       : "<<(pass?"PASS":"FAIL")<<"\n";

    return pass?0:1;
}
