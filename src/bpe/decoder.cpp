#include "bpe/decoder.hpp"

#include <limits>
#include <string>

#include "bpe/byte_fallback.hpp"
#include "core/error.hpp"
#include "core/status.hpp"

namespace qualix::bpe
{

Result<std::string>
BpeDecoder::Decode(
    std::span<const SymbolId> symbols,
    const Vocabulary& vocabulary
)
{
    std::string output;

    usize total_size = 0;

    /*
     * Validate every symbol before constructing
     * output.
     *
     * Normal vocabulary symbols contribute their
     * stored byte length.
     *
     * Byte fallback symbols such as:
     *
     *   <0x41>
     *   <0xE0>
     *
     * represent exactly one raw byte.
     */
    for (const SymbolId id : symbols)
    {
        if (id == InvalidSymbolId)
        {
            return Status{
                ErrorCode::VocabularyCorrupted
            };
        }

        const auto symbol =
            vocabulary.Find(id);

        if (!symbol.has_value())
        {
            return Status{
                ErrorCode::VocabularyCorrupted
            };
        }

        u8 byte = 0;

        const usize contribution =
            ByteFallback::ParseSymbol(
                *symbol,
                byte
            )
                ? 1
                : symbol->size();

        if (contribution >
            std::numeric_limits<usize>::max() -
            total_size)
        {
            return Status{
                ErrorCode::InvalidState
            };
        }

        total_size += contribution;
    }

    output.reserve(total_size);

    /*
     * Decode in symbol order.
     *
     * Learned BPE symbols already contain their
     * exact UTF-8 byte sequence and are appended
     * directly.
     *
     * Fallback symbols are converted back to the
     * single byte they represent.
     *
     * A sequence such as:
     *
     *   <0xE0> <0xAE> <0xA4>
     *
     * therefore reconstructs the original UTF-8
     * bytes exactly.
     */
    for (const SymbolId id : symbols)
    {
        const auto symbol =
            vocabulary.Find(id);

        /*
         * Already validated above.
         */
        u8 byte = 0;

        if (ByteFallback::ParseSymbol(
                *symbol,
                byte))
        {
            output.push_back(
                static_cast<char>(byte)
            );

            continue;
        }

        output.append(
            symbol->data(),
            symbol->size()
        );
    }

    return output;
}

Result<std::string>
BpeDecoder::Decode(
    std::span<const SymbolId> symbols,
    const BpeModel& model
)
{
    return Decode(
        symbols,
        model.GetVocabulary()
    );
}

} // namespace qualix::bpe
