#include <array>
#include <iostream>
#include <span>
#include <vector>

#include "bpe/binary_format.hpp"

using namespace qualix;
using namespace qualix::bpe;

namespace
{

usize tests_run = 0;
usize tests_passed = 0;

void Expect(
    bool condition,
    const char* name
)
{
    ++tests_run;

    if (condition)
    {
        ++tests_passed;

        std::cout
            << "[PASS] "
            << name
            << '\n';
    }
    else
    {
        std::cout
            << "[FAIL] "
            << name
            << '\n';
    }
}

} // namespace

int main()
{
    // ---------------------------------------------------------
    // Format constants
    // ---------------------------------------------------------

    {
        Expect(
            BpeBinaryMagic.size() == 8,
            "Magic has fixed size"
        );

        Expect(
            BpeBinaryMagic[0] == 'Q' &&
            BpeBinaryMagic[1] == 'L' &&
            BpeBinaryMagic[2] == 'X',
            "Magic prefix correct"
        );

        Expect(
            BpeBinaryVersion == 1,
            "Binary version is v1"
        );
    }

    // ---------------------------------------------------------
    // U32 little endian
    // ---------------------------------------------------------

    {
        BpeBinaryWriter writer;

        writer.WriteU32(
            0x12345678u
        );

        const auto& data =
            writer.Data();

        Expect(
            data.size() == 4,
            "U32 writes four bytes"
        );

        Expect(
            data ==
                std::vector<u8>{
                    0x78,
                    0x56,
                    0x34,
                    0x12
                },
            "U32 uses little endian"
        );

        BpeBinaryReader reader{
            data
        };

        u32 value = 0;

        Expect(
            reader.ReadU32(value),
            "U32 read succeeds"
        );

        Expect(
            value == 0x12345678u,
            "U32 round trip"
        );

        Expect(
            reader.End(),
            "Reader ends after U32"
        );
    }

    // ---------------------------------------------------------
    // U64 little endian
    // ---------------------------------------------------------

    {
        BpeBinaryWriter writer;

        writer.WriteU64(
            0x0123456789ABCDEFull
        );

        const auto& data =
            writer.Data();

        Expect(
            data ==
                std::vector<u8>{
                    0xEF,
                    0xCD,
                    0xAB,
                    0x89,
                    0x67,
                    0x45,
                    0x23,
                    0x01
                },
            "U64 uses little endian"
        );

        BpeBinaryReader reader{
            data
        };

        u64 value = 0;

        Expect(
            reader.ReadU64(value),
            "U64 read succeeds"
        );

        Expect(
            value ==
                0x0123456789ABCDEFull,
            "U64 round trip"
        );
    }

    // ---------------------------------------------------------
    // Raw bytes
    // ---------------------------------------------------------

    {
        BpeBinaryWriter writer;

        const std::array<u8, 5>
            input{
                1, 2, 3, 4, 5
            };

        writer.WriteBytes(
            input
        );

        BpeBinaryReader reader{
            writer.Data()
        };

        std::span<const u8> bytes;

        Expect(
            reader.ReadBytes(
                input.size(),
                bytes
            ),
            "Raw byte read succeeds"
        );

        Expect(
            bytes.size() ==
                input.size(),
            "Raw byte size preserved"
        );

        bool same = true;

        for (usize i = 0;
             i < input.size();
             ++i)
        {
            if (bytes[i] != input[i])
            {
                same = false;
                break;
            }
        }

        Expect(
            same,
            "Raw bytes preserved"
        );

        Expect(
            reader.End(),
            "Raw byte reader reaches end"
        );
    }

    // ---------------------------------------------------------
    // Sequential values
    // ---------------------------------------------------------

    {
        BpeBinaryWriter writer;

        writer.WriteU32(10);
        writer.WriteU64(20);
        writer.WriteU32(30);

        BpeBinaryReader reader{
            writer.Data()
        };

        u32 first = 0;
        u64 second = 0;
        u32 third = 0;

        Expect(
            reader.ReadU32(first) &&
            reader.ReadU64(second) &&
            reader.ReadU32(third),
            "Sequential values readable"
        );

        Expect(
            first == 10 &&
            second == 20 &&
            third == 30,
            "Sequential values preserved"
        );

        Expect(
            reader.End(),
            "Sequential reader reaches end"
        );
    }

    // ---------------------------------------------------------
    // Truncation safety
    // ---------------------------------------------------------

    {
        const std::vector<u8>
            data{
                1, 2, 3
            };

        BpeBinaryReader reader{
            data
        };

        u32 value = 99;

        Expect(
            !reader.ReadU32(value),
            "Truncated U32 rejected"
        );

        Expect(
            reader.Position() == 0,
            "Failed U32 does not advance"
        );
    }

    {
        const std::vector<u8>
            data{
                1, 2, 3, 4,
                5, 6, 7
            };

        BpeBinaryReader reader{
            data
        };

        u64 value = 99;

        Expect(
            !reader.ReadU64(value),
            "Truncated U64 rejected"
        );

        Expect(
            reader.Position() == 0,
            "Failed U64 does not advance"
        );
    }

    {
        const std::vector<u8>
            data{
                1, 2, 3
            };

        BpeBinaryReader reader{
            data
        };

        std::span<const u8> bytes;

        Expect(
            !reader.ReadBytes(
                4,
                bytes
            ),
            "Oversized byte read rejected"
        );

        Expect(
            reader.Position() == 0,
            "Failed byte read does not advance"
        );
    }

    // ---------------------------------------------------------
    // Empty reader
    // ---------------------------------------------------------

    {
        const std::vector<u8> data;

        BpeBinaryReader reader{
            data
        };

        Expect(
            reader.End(),
            "Empty reader starts at end"
        );

        Expect(
            reader.Remaining() == 0,
            "Empty reader has zero remaining"
        );
    }

    // ---------------------------------------------------------
    // Take
    // ---------------------------------------------------------

    {
        BpeBinaryWriter writer;

        writer.WriteU32(42);

        auto data =
            writer.Take();

        Expect(
            data.size() == 4,
            "Writer Take returns bytes"
        );
    }

    const usize failed =
        tests_run -
        tests_passed;

    std::cout
        << "\n================================\n"
        << "Tests Run    : "
        << tests_run
        << '\n'
        << "Tests Passed : "
        << tests_passed
        << '\n'
        << "Tests Failed : "
        << failed
        << '\n'
        << "================================\n";

    return failed == 0 ? 0 : 1;
}
