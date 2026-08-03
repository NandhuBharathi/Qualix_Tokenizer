#include "bpe/deserializer.hpp"

#include "bpe/serializer.hpp"

namespace qualix::bpe
{

Result<BpeModel>
BpeModelDeserializer::Deserialize(
    std::span<const u8> data
)
{
    /*
     * BpeModelSerializer owns the canonical
     * Qualix BPE Binary Format V1 parser.
     *
     * Keep only one implementation of the
     * binary format so serializer/deserializer
     * behavior cannot drift apart.
     */
    return
        BpeModelSerializer::Deserialize(
            data
        );
}

} // namespace qualix::bpe
