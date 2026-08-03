#include <iostream>
#include <span>
#include <string>
#include <vector>

#include "bpe/decoder.hpp"
#include "bpe/model_io.hpp"
#include "bpe/model_trainer.hpp"
#include "bpe/trainer.hpp"
#include "bpe/vocabulary.hpp"
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
    /*
     * Basic vocabulary decoding.
     */
    {
        Vocabulary vocabulary;

        const SymbolId h =
            vocabulary.Add("h");

        const SymbolId e =
            vocabulary.Add("e");

        const SymbolId l =
            vocabulary.Add("l");

        const SymbolId o =
            vocabulary.Add("o");

        const std::vector<SymbolId> ids{
            h,
            e,
            l,
            l,
            o
        };

        auto decoded =
            BpeDecoder::Decode(
                ids,
                vocabulary
            );

        Expect(
            decoded.Ok() &&
            decoded.Value() == "hello",
            "Basic symbol decoding"
        );
    }

    /*
     * Merged symbols must decode to their exact
     * underlying byte sequence.
     */
    {
        Vocabulary vocabulary;

        const SymbolId h =
            vocabulary.Add("h");

        const SymbolId e =
            vocabulary.Add("e");

        const SymbolId he =
            vocabulary.AddMerged(
                h,
                e
            );

        const SymbolId l =
            vocabulary.Add("l");

        const SymbolId o =
            vocabulary.Add("o");

        const std::vector<SymbolId> ids{
            he,
            l,
            l,
            o
        };

        auto decoded =
            BpeDecoder::Decode(
                ids,
                vocabulary
            );

        Expect(
            decoded.Ok() &&
            decoded.Value() == "hello",
            "Merged symbol decoding"
        );
    }

    /*
     * Tamil must remain byte exact.
     */
    {
        Vocabulary vocabulary;

        const SymbolId tamil =
            vocabulary.Add("தமிழ்");

        const std::vector<SymbolId> ids{
            tamil
        };

        auto decoded =
            BpeDecoder::Decode(
                ids,
                vocabulary
            );

        Expect(
            decoded.Ok() &&
            decoded.Value() == "தமிழ்",
            "Tamil decoding"
        );
    }

    /*
     * Emoji sequences must remain byte exact.
     */
    {
        Vocabulary vocabulary;

        const SymbolId emoji =
            vocabulary.Add("❤️");

        const std::vector<SymbolId> ids{
            emoji
        };

        auto decoded =
            BpeDecoder::Decode(
                ids,
                vocabulary
            );

        Expect(
            decoded.Ok() &&
            decoded.Value() == "❤️",
            "Emoji decoding"
        );
    }

    /*
     * Mixed Unicode.
     */
    {
        Vocabulary vocabulary;

        const SymbolId english =
            vocabulary.Add("Hello ");

        const SymbolId tamil =
            vocabulary.Add("தமிழ் ");

        const SymbolId emoji =
            vocabulary.Add("❤️");

        const std::vector<SymbolId> ids{
            english,
            tamil,
            emoji
        };

        auto decoded =
            BpeDecoder::Decode(
                ids,
                vocabulary
            );

        Expect(
            decoded.Ok() &&
            decoded.Value() ==
                "Hello தமிழ் ❤️",
            "Mixed Unicode decoding"
        );
    }

    /*
     * Empty input must decode to an empty string.
     */
    {
        Vocabulary vocabulary;

        const std::vector<SymbolId> ids;

        auto decoded =
            BpeDecoder::Decode(
                ids,
                vocabulary
            );

        Expect(
            decoded.Ok() &&
            decoded.Value().empty(),
            "Empty sequence decoding"
        );
    }

    /*
     * InvalidSymbolId is permanently reserved and
     * must never decode.
     */
    {
        Vocabulary vocabulary;

        const SymbolId a =
            vocabulary.Add("a");

        (void)a;

        const std::vector<SymbolId> ids{
            InvalidSymbolId
        };

        auto decoded =
            BpeDecoder::Decode(
                ids,
                vocabulary
            );

        Expect(
            decoded.Failed(),
            "Invalid symbol ID rejected"
        );
    }

    /*
     * Unknown IDs must be rejected.
     */
    {
        Vocabulary vocabulary;

        const SymbolId a =
            vocabulary.Add("a");

        (void)a;

        const std::vector<SymbolId> ids{
            999
        };

        auto decoded =
            BpeDecoder::Decode(
                ids,
                vocabulary
            );

        Expect(
            decoded.Failed(),
            "Unknown symbol ID rejected"
        );
    }

    /*
     * Train -> Encode -> Decode.
     *
     * Use text whose complete input is handled by
     * the BPE model's current Encode path.
     */
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

    {
        const std::string input =
            "hello";

        auto encoded =
            trained.Value().Encode(
                input
            );

        Expect(
            encoded.Ok(),
            "English encoding succeeds"
        );

        if (encoded.Ok())
        {
            auto decoded =
                BpeDecoder::Decode(
                    encoded.Value(),
                    trained.Value()
                );

            Expect(
                decoded.Ok() &&
                decoded.Value() == input,
                "English encode-decode round trip"
            );
        }
    }

    {
        const std::string input =
            "தமிழ்";

        auto encoded =
            trained.Value().Encode(
                input
            );

        Expect(
            encoded.Ok(),
            "Tamil encoding succeeds"
        );

        if (encoded.Ok())
        {
            auto decoded =
                BpeDecoder::Decode(
                    encoded.Value(),
                    trained.Value()
                );

            Expect(
                decoded.Ok() &&
                decoded.Value() == input,
                "Tamil encode-decode round trip"
            );
        }
    }

    /*
     * Persistence:
     *
     * Train -> Save -> Load -> Encode -> Decode.
     */
    {
        const auto path =
            std::filesystem::temp_directory_path() /
            "qualix_bpe_decoder_test.qlxbpe";

        const Status saved =
            BpeModelIO::Save(
                trained.Value(),
                path
            );

        Expect(
            saved.Ok(),
            "Model save succeeds"
        );

        if (saved.Ok())
        {
            auto loaded =
                BpeModelIO::Load(
                    path
                );

            Expect(
                loaded.Ok(),
                "Model load succeeds"
            );

            if (loaded.Ok())
            {
                const std::string input =
                    "token";

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
                        BpeDecoder::Decode(
                            encoded.Value(),
                            loaded.Value()
                        );

                    Expect(
                        decoded.Ok() &&
                        decoded.Value() == input,
                        "Loaded model decode round trip"
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
