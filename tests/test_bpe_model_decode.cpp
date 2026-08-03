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

} // namespace

int main()
{
    const TrainerConfig config{
        64,
        2
    };

    const std::string corpus =
        "hello hello hello "
        "token token token "
        "தமிழ் தமிழ் தமிழ்";

    auto trained =
        BpeModelTrainer::Train(
            corpus,
            config
        );

    Expect(
        trained.Ok(),
        "Model training succeeds"
    );

    if (trained.Failed())
        return 1;

    /*
     * English:
     *
     * Model.Encode -> Model.Decode
     */
    {
        const std::string input =
            "hello";

        auto encoded =
            trained.Value().Encode(
                input
            );

        Expect(
            encoded.Ok(),
            "Model English encoding succeeds"
        );

        if (encoded.Ok())
        {
            auto decoded =
                trained.Value().Decode(
                    encoded.Value()
                );

            Expect(
                decoded.Ok(),
                "Model English decoding succeeds"
            );

            if (decoded.Ok())
            {
                Expect(
                    decoded.Value() == input,
                    "Model English round trip exact"
                );
            }
        }
    }

    /*
     * Tamil:
     *
     * Exact UTF-8 bytes must survive.
     */
    {
        const std::string input =
            "தமிழ்";

        auto encoded =
            trained.Value().Encode(
                input
            );

        Expect(
            encoded.Ok(),
            "Model Tamil encoding succeeds"
        );

        if (encoded.Ok())
        {
            auto decoded =
                trained.Value().Decode(
                    encoded.Value()
                );

            Expect(
                decoded.Ok(),
                "Model Tamil decoding succeeds"
            );

            if (decoded.Ok())
            {
                Expect(
                    decoded.Value() == input,
                    "Model Tamil round trip exact"
                );
            }
        }
    }

    /*
     * Another learned word.
     */
    {
        const std::string input =
            "token";

        auto encoded =
            trained.Value().Encode(
                input
            );

        Expect(
            encoded.Ok(),
            "Model token encoding succeeds"
        );

        if (encoded.Ok())
        {
            auto decoded =
                trained.Value().Decode(
                    encoded.Value()
                );

            Expect(
                decoded.Ok() &&
                decoded.Value() == input,
                "Model token round trip exact"
            );
        }
    }

    /*
     * Empty symbol sequence.
     */
    {
        const std::vector<SymbolId> empty;

        auto decoded =
            trained.Value().Decode(
                empty
            );

        Expect(
            decoded.Ok() &&
            decoded.Value().empty(),
            "Model empty decode succeeds"
        );
    }

    /*
     * InvalidSymbolId must fail.
     */
    {
        const std::vector<SymbolId> ids{
            InvalidSymbolId
        };

        auto decoded =
            trained.Value().Decode(
                ids
            );

        Expect(
            decoded.Failed(),
            "Model rejects InvalidSymbolId"
        );
    }

    /*
     * Unknown ID must fail.
     */
    {
        const SymbolId unknown =
            static_cast<SymbolId>(
                trained.Value().VocabularySize() +
                100
            );

        const std::vector<SymbolId> ids{
            unknown
        };

        auto decoded =
            trained.Value().Decode(
                ids
            );

        Expect(
            decoded.Failed(),
            "Model rejects unknown symbol ID"
        );
    }

    /*
     * Persistence integration:
     *
     * Train
     *   -> Save
     *   -> Load
     *   -> Encode
     *   -> Model.Decode
     */
    {
        const auto path =
            std::filesystem::temp_directory_path() /
            "qualix_bpe_model_decode_test.qlxbpe";

        const Status saved =
            BpeModelIO::Save(
                trained.Value(),
                path
            );

        Expect(
            saved.Ok(),
            "Model persistence save succeeds"
        );

        if (saved.Ok())
        {
            auto loaded =
                BpeModelIO::Load(
                    path
                );

            Expect(
                loaded.Ok(),
                "Model persistence load succeeds"
            );

            if (loaded.Ok())
            {
                const std::string input =
                    "hello";

                auto encoded =
                    loaded.Value().Encode(
                        input
                    );

                Expect(
                    encoded.Ok(),
                    "Loaded model encoding succeeds"
                );

                if (encoded.Ok())
                {
                    auto decoded =
                        loaded.Value().Decode(
                            encoded.Value()
                        );

                    Expect(
                        decoded.Ok() &&
                        decoded.Value() == input,
                        "Loaded model Decode API round trip"
                    );
                }
            }
        }

        std::error_code ec;

        std::filesystem::remove(
            path,
            ec
        );
    }

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
