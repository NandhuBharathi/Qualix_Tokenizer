#pragma once

#include <span>
#include <vector>

#include "bpe/model.hpp"
#include "core/result.hpp"
#include "core/types.hpp"

namespace qualix::bpe
{

class BpeModelSerializer
{
public:
    [[nodiscard]]
    static Result<std::vector<u8>> Serialize(
        const BpeModel& model
    );

    [[nodiscard]]
    static Result<BpeModel> Deserialize(
        std::span<const u8> data
    );
};

} // namespace qualix::bpe
