#include "bpe/model_io.hpp"

#include <fstream>
#include <limits>
#include <vector>

#include "bpe/serializer.hpp"
#include "core/error.hpp"
#include "core/types.hpp"

namespace qualix::bpe
{

Status BpeModelIO::Save(
    const BpeModel& model,
    const std::filesystem::path& path
)
{
    if (path.empty())
    {
        return Status{
            ErrorCode::InvalidArgument
        };
    }

    auto serialized =
        BpeModelSerializer::Serialize(
            model
        );

    if (serialized.Failed())
        return serialized.GetStatus();

    std::ofstream file{
        path,
        std::ios::binary |
        std::ios::trunc
    };

    if (!file.is_open())
    {
        return Status{
            ErrorCode::FileOpenFailed
        };
    }

    const auto& bytes =
        serialized.Value();

    if (!bytes.empty())
    {
        if (bytes.size() >
            static_cast<usize>(
                std::numeric_limits<
                    std::streamsize
                >::max()
            ))
        {
            return Status{
                ErrorCode::FileWriteFailed
            };
        }

        file.write(
            reinterpret_cast<
                const char*
            >(bytes.data()),
            static_cast<std::streamsize>(
                bytes.size()
            )
        );

        if (!file)
        {
            return Status{
                ErrorCode::FileWriteFailed
            };
        }
    }

    file.flush();

    if (!file)
    {
        return Status{
            ErrorCode::FileWriteFailed
        };
    }

    return Status::Success();
}

Result<BpeModel> BpeModelIO::Load(
    const std::filesystem::path& path
)
{
    if (path.empty())
    {
        return Status{
            ErrorCode::InvalidArgument
        };
    }

    std::ifstream file{
        path,
        std::ios::binary |
        std::ios::ate
    };

    if (!file.is_open())
    {
        if (!std::filesystem::exists(path))
        {
            return Status{
                ErrorCode::FileNotFound
            };
        }

        return Status{
            ErrorCode::FileOpenFailed
        };
    }

    const std::streampos end =
        file.tellg();

    if (end < 0)
    {
        return Status{
            ErrorCode::FileReadFailed
        };
    }

    const auto size =
        static_cast<u64>(end);

    if (size >
        static_cast<u64>(
            std::numeric_limits<
                usize
            >::max()
        ))
    {
        return Status{
            ErrorCode::FileReadFailed
        };
    }

    file.seekg(
        0,
        std::ios::beg
    );

    if (!file)
    {
        return Status{
            ErrorCode::FileReadFailed
        };
    }

    std::vector<u8> bytes(
        static_cast<usize>(size)
    );

    if (!bytes.empty())
    {
        if (bytes.size() >
            static_cast<usize>(
                std::numeric_limits<
                    std::streamsize
                >::max()
            ))
        {
            return Status{
                ErrorCode::FileReadFailed
            };
        }

        file.read(
            reinterpret_cast<char*>(
                bytes.data()
            ),
            static_cast<std::streamsize>(
                bytes.size()
            )
        );

        if (!file)
        {
            return Status{
                ErrorCode::FileReadFailed
            };
        }
    }

    return
        BpeModelSerializer::Deserialize(
            bytes
        );
}

} // namespace qualix::bpe
