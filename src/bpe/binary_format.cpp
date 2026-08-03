#include "bpe/binary_format.hpp"

#include <utility>

namespace qualix::bpe
{

void BpeBinaryWriter::WriteU32(
    u32 value
)
{
    for (unsigned int shift = 0;
         shift < 32;
         shift += 8)
    {
        data_.push_back(
            static_cast<u8>(
                (value >> shift) &
                0xFFu
            )
        );
    }
}

void BpeBinaryWriter::WriteU64(
    u64 value
)
{
    for (unsigned int shift = 0;
         shift < 64;
         shift += 8)
    {
        data_.push_back(
            static_cast<u8>(
                (value >> shift) &
                0xFFu
            )
        );
    }
}

void BpeBinaryWriter::WriteBytes(
    std::span<const u8> bytes
)
{
    data_.insert(
        data_.end(),
        bytes.begin(),
        bytes.end()
    );
}

const std::vector<u8>&
BpeBinaryWriter::Data() const noexcept
{
    return data_;
}

std::vector<u8>
BpeBinaryWriter::Take()
{
    return std::move(data_);
}

BpeBinaryReader::BpeBinaryReader(
    std::span<const u8> data
) noexcept
    : data_(data)
{
}

bool BpeBinaryReader::ReadU32(
    u32& value
) noexcept
{
    if (Remaining() < 4)
        return false;

    value = 0;

    for (unsigned int i = 0;
         i < 4;
         ++i)
    {
        value |=
            static_cast<u32>(
                data_[position_ + i]
            ) << (i * 8);
    }

    position_ += 4;

    return true;
}

bool BpeBinaryReader::ReadU64(
    u64& value
) noexcept
{
    if (Remaining() < 8)
        return false;

    value = 0;

    for (unsigned int i = 0;
         i < 8;
         ++i)
    {
        value |=
            static_cast<u64>(
                data_[position_ + i]
            ) << (i * 8);
    }

    position_ += 8;

    return true;
}

bool BpeBinaryReader::ReadBytes(
    usize count,
    std::span<const u8>& bytes
) noexcept
{
    if (count > Remaining())
        return false;

    bytes = data_.subspan(
        position_,
        count
    );

    position_ += count;

    return true;
}

usize
BpeBinaryReader::Position() const noexcept
{
    return position_;
}

usize
BpeBinaryReader::Remaining() const noexcept
{
    return
        data_.size() -
        position_;
}

bool
BpeBinaryReader::End() const noexcept
{
    return
        position_ ==
        data_.size();
}

} // namespace qualix::bpe
