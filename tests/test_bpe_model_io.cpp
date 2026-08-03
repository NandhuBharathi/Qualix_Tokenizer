#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "bpe/model_io.hpp"
#include "bpe/model_trainer.hpp"
#include "bpe/trainer.hpp"
#include "core/types.hpp"

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

bool SameVocabulary(
    const BpeModel& a,
    const BpeModel& b
)
{
    if (a.VocabularySize() !=
        b.VocabularySize())
    {
        return false;
    }

    for (usize i = 1;
         i <= a.VocabularySize();
         ++i)
    {
        const auto left =
            a.GetVocabulary().Find(
                static_cast<SymbolId>(i)
            );

        const auto right =
            b.GetVocabulary().Find(
                static_cast<SymbolId>(i)
            );

        if (!left.has_value() ||
            !right.has_value() ||
            *left != *right)
        {
            return false;
        }
    }

    return true;
}

} // namespace

int main()
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() /
        "qualix_bpe_model_io_test.qlxbpe";

    const std::filesystem::path
        corrupt_path =
            std::filesystem::temp_directory_path() /
            "qualix_bpe_model_io_corrupt.qlxbpe";

    const std::filesystem::path
        missing_path =
            std::filesystem::temp_directory_path() /
            "qualix_bpe_model_io_missing.qlxbpe";

    std::error_code ec;

    std::filesystem::remove(
        path,
        ec
    );

    std::filesystem::remove(
        corrupt_path,
        ec
    );

    std::filesystem::remove(
        missing_path,
        ec
    );

    const TrainerConfig config{
        64,
        2
    };

    const std::string corpus =
        "hello hello hello "
        "token tokenizer token "
        "தமிழ் தமிழ் தமிழ் "
        "qualix qualix qualix";

    auto trained =
        BpeModelTrainer::Train(
            corpus,
            config
        );

    Expect(
        trained.Ok(),
        "Training succeeds"
    );

    if (trained.Failed())
        return 1;

    const Status saved =
        BpeModelIO::Save(
            trained.Value(),
            path
        );

    Expect(
        saved.Ok(),
        "Model file save succeeds"
    );

    Expect(
        std::filesystem::exists(path),
        "Model file created"
    );

    if (std::filesystem::exists(path))
    {
        Expect(
            std::filesystem::file_size(
                path
            ) > 0,
            "Model file non-empty"
        );
    }

    auto loaded =
        BpeModelIO::Load(
            path
        );

    Expect(
        loaded.Ok(),
        "Model file load succeeds"
    );

    if (loaded.Ok())
    {
        Expect(
            SameVocabulary(
                trained.Value(),
                loaded.Value()
            ),
            "Vocabulary exact IDs preserved"
        );

        Expect(
            trained.Value().GetRules() ==
            loaded.Value().GetRules(),
            "Merge rules preserved"
        );

        Expect(
            trained.Value().VocabularySize() ==
            loaded.Value().VocabularySize(),
            "Vocabulary size preserved"
        );

        Expect(
            trained.Value().RuleCount() ==
            loaded.Value().RuleCount(),
            "Rule count preserved"
        );

        auto original =
            trained.Value().Encode(
                "hello"
            );

        auto restored =
            loaded.Value().Encode(
                "hello"
            );

        Expect(
            original.Ok() &&
            restored.Ok() &&
            original.Value() ==
            restored.Value(),
            "Encoding preserved after file round trip"
        );
    }

    /*
     * Empty models must also survive disk
     * persistence.
     */

    {
        BpeModel empty;

        const Status status =
            BpeModelIO::Save(
                empty,
                path
            );

        Expect(
            status.Ok(),
            "Empty model file save"
        );

        auto restored =
            BpeModelIO::Load(
                path
            );

        Expect(
            restored.Ok() &&
            restored.Value().Empty(),
            "Empty model file round trip"
        );
    }

    /*
     * Missing file.
     */

    {
        auto result =
            BpeModelIO::Load(
                missing_path
            );

        Expect(
            result.Failed(),
            "Missing model file rejected"
        );
    }

    /*
     * Corrupted file.
     */

    {
        std::ofstream file{
            corrupt_path,
            std::ios::binary |
            std::ios::trunc
        };

        const char garbage[] = {
            'B', 'A', 'D'
        };

        file.write(
            garbage,
            sizeof(garbage)
        );

        file.close();

        auto result =
            BpeModelIO::Load(
                corrupt_path
            );

        Expect(
            result.Failed(),
            "Corrupted model file rejected"
        );
    }

    /*
     * Empty path.
     */

    {
        BpeModel empty;

        const Status status =
            BpeModelIO::Save(
                empty,
                {}
            );

        Expect(
            status.Failed(),
            "Empty save path rejected"
        );

        auto loaded =
            BpeModelIO::Load(
                {}
            );

        Expect(
            loaded.Failed(),
            "Empty load path rejected"
        );
    }

    std::filesystem::remove(
        path,
        ec
    );

    std::filesystem::remove(
        corrupt_path,
        ec
    );

    std::cout
        << '\n'
        << tests_passed
        << "/"
        << tests_run
        << " tests passed\n";

    return
        tests_passed == tests_run
            ? 0
            : 1;
}
