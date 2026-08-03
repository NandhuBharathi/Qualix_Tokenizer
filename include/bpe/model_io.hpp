#pragma once

#include <filesystem>

#include "bpe/model.hpp"
#include "core/result.hpp"
#include "core/status.hpp"

namespace qualix::bpe
{

class BpeModelIO
{
public:
    [[nodiscard]]
    static Status Save(
        const BpeModel& model,
        const std::filesystem::path& path
    );

    [[nodiscard]]
    static Result<BpeModel> Load(
        const std::filesystem::path& path
    );
};

} // namespace qualix::bpe
