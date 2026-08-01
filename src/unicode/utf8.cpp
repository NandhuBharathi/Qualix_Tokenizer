#include "unicode/utf8.hpp"

namespace qualix::unicode
{

usize Utf8::SequenceLength(u8 lead_byte) noexcept
{
    if (lead_byte <= 0x7F)
        return 1;

    if (lead_byte >= 0xC2 && lead_byte <= 0xDF)
        return 2;

    if (lead_byte >= 0xE0 && lead_byte <= 0xEF)
        return 3;

    if (lead_byte >= 0xF0 && lead_byte <= 0xF4)
        return 4;

    return 0;
}

Result<Utf8DecodeResult> Utf8::Decode(std::string_view input) noexcept
{
    if (input.empty())
        return Status::Failure(ErrorCode::InvalidUtf8);

    const auto b0 = static_cast<u8>(
        static_cast<unsigned char>(input[0])
    );

    const usize length = SequenceLength(b0);

    if (length == 0 || input.size() < length)
        return Status::Failure(ErrorCode::InvalidUtf8);

    if (length == 1)
    {
        return Utf8DecodeResult{
            static_cast<CodePoint>(b0),
            1
        };
    }

    const auto b1 = static_cast<u8>(
        static_cast<unsigned char>(input[1])
    );

    if (!IsContinuationByte(b1))
        return Status::Failure(ErrorCode::InvalidUtf8);

    if (length == 2)
    {
        const CodePoint cp =
            (static_cast<CodePoint>(b0 & 0x1F) << 6) |
            static_cast<CodePoint>(b1 & 0x3F);

        if (!IsValidCodePoint(cp))
            return Status::Failure(ErrorCode::InvalidUtf8);

        return Utf8DecodeResult{cp, 2};
    }

    const auto b2 = static_cast<u8>(
        static_cast<unsigned char>(input[2])
    );

    if (!IsContinuationByte(b2))
        return Status::Failure(ErrorCode::InvalidUtf8);

    if (b0 == 0xE0 && b1 < 0xA0)
        return Status::Failure(ErrorCode::InvalidUtf8);

    if (b0 == 0xED && b1 > 0x9F)
        return Status::Failure(ErrorCode::InvalidUtf8);

    if (length == 3)
    {
        const CodePoint cp =
            (static_cast<CodePoint>(b0 & 0x0F) << 12) |
            (static_cast<CodePoint>(b1 & 0x3F) << 6) |
            static_cast<CodePoint>(b2 & 0x3F);

        if (!IsValidCodePoint(cp))
            return Status::Failure(ErrorCode::InvalidUtf8);

        return Utf8DecodeResult{cp, 3};
    }

    const auto b3 = static_cast<u8>(
        static_cast<unsigned char>(input[3])
    );

    if (!IsContinuationByte(b3))
        return Status::Failure(ErrorCode::InvalidUtf8);

    if (b0 == 0xF0 && b1 < 0x90)
        return Status::Failure(ErrorCode::InvalidUtf8);

    if (b0 == 0xF4 && b1 > 0x8F)
        return Status::Failure(ErrorCode::InvalidUtf8);

    const CodePoint cp =
        (static_cast<CodePoint>(b0 & 0x07) << 18) |
        (static_cast<CodePoint>(b1 & 0x3F) << 12) |
        (static_cast<CodePoint>(b2 & 0x3F) << 6) |
        static_cast<CodePoint>(b3 & 0x3F);

    if (!IsValidCodePoint(cp))
        return Status::Failure(ErrorCode::InvalidUtf8);

    return Utf8DecodeResult{cp, 4};
}

bool Utf8::Validate(std::string_view input) noexcept
{
    usize offset = 0;

    while (offset < input.size())
    {
        const auto result = Decode(input.substr(offset));

        if (result.Failed())
            return false;

        offset += result.Value().bytes_consumed;
    }

    return true;
}

Result<std::string> Utf8::Encode(CodePoint codepoint)
{
    if (!IsValidCodePoint(codepoint))
        return Status::Failure(ErrorCode::InvalidCodePoint);

    std::string output;

    if (codepoint <= 0x7F)
    {
        output.push_back(static_cast<char>(codepoint));
    }
    else if (codepoint <= 0x7FF)
    {
        output.push_back(static_cast<char>(
            0xC0 | ((codepoint >> 6) & 0x1F)
        ));

        output.push_back(static_cast<char>(
            0x80 | (codepoint & 0x3F)
        ));
    }
    else if (codepoint <= 0xFFFF)
    {
        output.push_back(static_cast<char>(
            0xE0 | ((codepoint >> 12) & 0x0F)
        ));

        output.push_back(static_cast<char>(
            0x80 | ((codepoint >> 6) & 0x3F)
        ));

        output.push_back(static_cast<char>(
            0x80 | (codepoint & 0x3F)
        ));
    }
    else
    {
        output.push_back(static_cast<char>(
            0xF0 | ((codepoint >> 18) & 0x07)
        ));

        output.push_back(static_cast<char>(
            0x80 | ((codepoint >> 12) & 0x3F)
        ));

        output.push_back(static_cast<char>(
            0x80 | ((codepoint >> 6) & 0x3F)
        ));

        output.push_back(static_cast<char>(
            0x80 | (codepoint & 0x3F)
        ));
    }

    return output;
}

} // namespace qualix::unicode
