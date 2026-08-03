#include "bpe/serializer.hpp"

#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "bpe/binary_format.hpp"
#include "core/error.hpp"
#include "core/status.hpp"
#include "unicode/utf8.hpp"

namespace qualix::bpe
{

namespace
{

Status Corrupted()
{
    return Status{
        ErrorCode::VocabularyCorrupted
    };
}

bool WriteHeader(
    BpeBinaryWriter& writer
)
{
    writer.WriteBytes(
        std::span<const u8>{
            BpeBinaryMagic.data(),
            BpeBinaryMagic.size()
        }
    );

    writer.WriteU32(
        BpeBinaryVersion
    );

    writer.WriteU32(
        BpeBinaryFlags
    );

    return true;
}

bool ReadHeader(
    BpeBinaryReader& reader
)
{
    std::span<const u8> magic;

    if (!reader.ReadBytes(
            BpeBinaryMagic.size(),
            magic))
    {
        return false;
    }

    for (usize i = 0;
         i < BpeBinaryMagic.size();
         ++i)
    {
        if (magic[i] !=
            BpeBinaryMagic[i])
        {
            return false;
        }
    }

    u32 version = 0;
    u32 flags = 0;

    if (!reader.ReadU32(version))
        return false;

    if (!reader.ReadU32(flags))
        return false;

    if (version !=
        BpeBinaryVersion)
    {
        return false;
    }

    if (flags !=
        BpeBinaryFlags)
    {
        return false;
    }

    return true;
}

bool ValidUtf8(
    std::string_view text
)
{
    const auto result =
        unicode::Utf8::Decode(text);

    return result.Ok();
}

} // namespace

Result<std::vector<u8>>
BpeModelSerializer::Serialize(
    const BpeModel& model
)
{
    BpeBinaryWriter writer;

    WriteHeader(writer);

    const auto& vocabulary =
        model.GetVocabulary();

    const usize vocabulary_size =
        vocabulary.Size();

    if (vocabulary_size >
        std::numeric_limits<u32>::max())
    {
        return Status{
            ErrorCode::InvalidState
        };
    }

    writer.WriteU32(
        static_cast<u32>(
            vocabulary_size
        )
    );

    /*
     * Vocabulary IDs are dense and stable:
     *
     * 1, 2, 3, ... N
     *
     * We still serialize the ID explicitly so
     * the binary format remains self-validating
     * and exact-ID reconstruction is guaranteed.
     */
    for (usize index = 1;
         index <= vocabulary_size;
         ++index)
    {
        const SymbolId id =
            static_cast<SymbolId>(
                index
            );

        const auto symbol =
            vocabulary.Find(id);

        if (!symbol.has_value() ||
            symbol->empty())
        {
            return Status{
                ErrorCode::InvalidState
            };
        }

        if (symbol->size() >
            std::numeric_limits<u32>::max())
        {
            return Status{
                ErrorCode::InvalidState
            };
        }

        writer.WriteU32(id);

        writer.WriteU32(
            static_cast<u32>(
                symbol->size()
            )
        );

        writer.WriteBytes(
            std::span<const u8>{
                reinterpret_cast<
                    const u8*
                >(symbol->data()),
                symbol->size()
            }
        );
    }

    const auto& rules =
        model.GetRules();

    if (rules.size() >
        std::numeric_limits<u32>::max())
    {
        return Status{
            ErrorCode::InvalidState
        };
    }

    writer.WriteU32(
        static_cast<u32>(
            rules.size()
        )
    );

    for (const auto& rule : rules)
    {
        if (!rule.Valid())
        {
            return Status{
                ErrorCode::InvalidState
            };
        }

        if (!vocabulary.Contains(
                rule.pair.left) ||
            !vocabulary.Contains(
                rule.pair.right) ||
            !vocabulary.Contains(
                rule.merged_symbol))
        {
            return Status{
                ErrorCode::InvalidState
            };
        }

        if (rule.rank >
            std::numeric_limits<u64>::max())
        {
            return Status{
                ErrorCode::InvalidState
            };
        }

        writer.WriteU32(
            rule.pair.left
        );

        writer.WriteU32(
            rule.pair.right
        );

        writer.WriteU32(
            rule.merged_symbol
        );

        writer.WriteU64(
            static_cast<u64>(
                rule.rank
            )
        );
    }

    return writer.Take();
}

Result<BpeModel>
BpeModelSerializer::Deserialize(
    std::span<const u8> data
)
{
    BpeBinaryReader reader{
        data
    };

    if (!ReadHeader(reader))
        return Corrupted();

    u32 vocabulary_count = 0;

    if (!reader.ReadU32(
            vocabulary_count))
    {
        return Corrupted();
    }

    BpeModel model;

    auto& vocabulary =
        model.GetVocabulary();

    /*
     * Every vocabulary entry requires at least:
     *
     * id     : u32
     * length : u32
     *
     * Reject impossible counts before allocation
     * or iteration.
     */
    if (static_cast<u64>(
            vocabulary_count) >
        static_cast<u64>(
            reader.Remaining()) / 8u)
    {
        return Corrupted();
    }

    for (u32 index = 0;
         index < vocabulary_count;
         ++index)
    {
        u32 id = 0;
        u32 length = 0;

        if (!reader.ReadU32(id))
            return Corrupted();

        if (!reader.ReadU32(length))
            return Corrupted();

        /*
         * Dense vocabulary invariant.
         */
        const SymbolId expected =
            static_cast<SymbolId>(
                index + 1
            );

        if (id != expected)
            return Corrupted();

        if (length == 0)
            return Corrupted();

        std::span<const u8> bytes;

        if (!reader.ReadBytes(
                static_cast<usize>(
                    length
                ),
                bytes))
        {
            return Corrupted();
        }

        std::string symbol{
            reinterpret_cast<
                const char*
            >(bytes.data()),
            bytes.size()
        };

        if (!ValidUtf8(symbol))
            return Corrupted();

        if (!vocabulary.AddWithId(
                id,
                std::move(symbol)))
        {
            return Corrupted();
        }
    }

    u32 rule_count = 0;

    if (!reader.ReadU32(
            rule_count))
    {
        return Corrupted();
    }

    /*
     * Each V1 rule is exactly:
     *
     * left   u32
     * right  u32
     * merged u32
     * rank   u64
     *
     * = 20 bytes
     */
    constexpr usize
        RuleBytes = 20;

    if (static_cast<u64>(
            rule_count) >
        static_cast<u64>(
            reader.Remaining()) /
            RuleBytes)
    {
        return Corrupted();
    }

    std::vector<MergeRule> rules;

    rules.reserve(
        rule_count
    );

    for (u32 i = 0;
         i < rule_count;
         ++i)
    {
        u32 left = 0;
        u32 right = 0;
        u32 merged = 0;
        u64 rank = 0;

        if (!reader.ReadU32(left) ||
            !reader.ReadU32(right) ||
            !reader.ReadU32(merged) ||
            !reader.ReadU64(rank))
        {
            return Corrupted();
        }

        if (!vocabulary.Contains(left) ||
            !vocabulary.Contains(right) ||
            !vocabulary.Contains(merged))
        {
            return Corrupted();
        }

        if (rank >
            static_cast<u64>(
                std::numeric_limits<
                    MergeRank
                >::max()))
        {
            return Corrupted();
        }

        const MergeRule rule{
            Pair{
                left,
                right
            },
            merged,
            static_cast<MergeRank>(
                rank
            )
        };

        if (!rule.Valid())
            return Corrupted();

        /*
         * A merge rule is semantically valid only
         * when its merged symbol contains exactly
         * the concatenation represented by:
         *
         *     left + right
         *
         * Merely checking that all three IDs exist
         * is insufficient for a persisted model.
         */
        const auto left_symbol =
            vocabulary.Find(
                rule.pair.left
            );

        const auto right_symbol =
            vocabulary.Find(
                rule.pair.right
            );

        const auto merged_symbol =
            vocabulary.Find(
                rule.merged_symbol
            );

        if (!left_symbol.has_value() ||
            !right_symbol.has_value() ||
            !merged_symbol.has_value())
        {
            return Corrupted();
        }

        if (left_symbol->size() >
            std::numeric_limits<usize>::max() -
            right_symbol->size())
        {
            return Corrupted();
        }

        const usize expected_size =
            left_symbol->size() +
            right_symbol->size();

        if (merged_symbol->size() !=
            expected_size)
        {
            return Corrupted();
        }

        if (merged_symbol->compare(
                0,
                left_symbol->size(),
                left_symbol->data(),
                left_symbol->size()) != 0)
        {
            return Corrupted();
        }

        if (merged_symbol->compare(
                left_symbol->size(),
                right_symbol->size(),
                right_symbol->data(),
                right_symbol->size()) != 0)
        {
            return Corrupted();
        }

        /*
         * Trainer-generated rules have canonical
         * ranks:
         *
         *     0, 1, 2, ... N-1
         *
         * Reject reordered, duplicated or sparse
         * ranks in persisted models.
         */
        if (rule.rank !=
            static_cast<MergeRank>(i))
        {
            return Corrupted();
        }

        rules.push_back(
            rule
        );
    }

    /*
     * V1 does not permit trailing bytes.
     *
     * This catches accidental concatenation,
     * malformed payloads and unsupported data.
     */
    if (!reader.End())
        return Corrupted();

    model.SetRules(
        std::move(rules)
    );

    return model;
}

} // namespace qualix::bpe
