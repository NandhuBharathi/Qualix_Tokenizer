#pragma once

#include <string_view>

#include "bpe/model.hpp"
#include "bpe/trainer.hpp"
#include "core/result.hpp"

namespace qualix::bpe
{

class BpeModelTrainer
{
public:
    [[nodiscard]]
    static Result<BpeModel> Train(
        std::string_view input,
        const TrainerConfig& config
    );
};

} // namespace qualix::bpe
