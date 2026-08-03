#pragma once

#include <span>

#include "bpe/model.hpp"
#include "core/result.hpp"
#include "core/types.hpp"

namespace qualix::bpe
{

class BpeModelDeserializer
{
public:
    [[nodiscard]]
    static Result<BpeModel> Deserialize(
        std::span<const u8> data
    );
};

} // namespace qualix::bpe
