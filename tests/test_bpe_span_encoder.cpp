#include <iostream>
#include <string>
#include <vector>

#include "bpe/span_encoder.hpp"

using namespace qualix;
using namespace qualix::bpe;
using namespace qualix::pretokenizer;

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

std::string Decode(
    const EncodedSpan& span,
    const Vocabulary& vocabulary
)
{
    std::string output;

    for (const SymbolId id :
         span.symbols)
    {
        const auto symbol =
            vocabulary.Find(id);

        if (symbol.has_value())
            output += *symbol;
    }

    return output;
}

} // namespace

int main()
{
    {
        Vocabulary vocabulary;

        const auto result =
            BpeSpanEncoder::Encode(
                "",
                vocabulary
            );

        Expect(
            result.Ok(),
            "Empty input succeeds"
        );

        Expect(
            result.Value().empty(),
            "Empty input has no spans"
        );
    }

    {
        Vocabulary vocabulary;

        const auto result =
            BpeSpanEncoder::Encode(
                "Hello world",
                vocabulary
            );

        Expect(
            result.Ok(),
            "Plain text encoding succeeds"
        );

        if (result.Ok())
        {
            const auto& spans =
                result.Value();

            Expect(
                spans.size() == 3,
                "Word space word boundaries preserved"
            );

            Expect(
                spans.size() == 3 &&
                spans[0].type ==
                    SpanType::Word &&
                spans[1].type ==
                    SpanType::Whitespace &&
                spans[2].type ==
                    SpanType::Word,
                "Span types preserved"
            );

            Expect(
                spans.size() == 3 &&
                Decode(
                    spans[0],
                    vocabulary
                ) == "Hello" &&
                Decode(
                    spans[1],
                    vocabulary
                ) == " " &&
                Decode(
                    spans[2],
                    vocabulary
                ) == "world",
                "Span text reconstructs exactly"
            );

            Expect(
                spans.size() == 3 &&
                !spans[0].protected_span &&
                !spans[1].protected_span &&
                !spans[2].protected_span,
                "Plain spans remain splittable"
            );
        }
    }

    {
        Vocabulary vocabulary;

        const std::string input =
            "ragubathi@gmail.com";

        const auto result =
            BpeSpanEncoder::Encode(
                input,
                vocabulary
            );

        Expect(
            result.Ok(),
            "Email encoding succeeds"
        );

        if (result.Ok())
        {
            const auto& spans =
                result.Value();

            Expect(
                spans.size() == 1,
                "Email remains one span boundary"
            );

            Expect(
                spans.size() == 1 &&
                spans[0].type ==
                    SpanType::Email,
                "Email type preserved"
            );

            Expect(
                spans.size() == 1 &&
                spans[0].protected_span,
                "Email protected flag preserved"
            );

            Expect(
                spans.size() == 1 &&
                Decode(
                    spans[0],
                    vocabulary
                ) == input,
                "Email internally symbolized losslessly"
            );

            Expect(
                spans.size() == 1 &&
                spans[0].symbols.size() > 1,
                "Protected email is not forced into one token"
            );
        }
    }

    {
        Vocabulary vocabulary;

        const std::string input =
            "Hi🙂Tamil";

        const auto result =
            BpeSpanEncoder::Encode(
                input,
                vocabulary
            );

        Expect(
            result.Ok(),
            "Mixed Unicode encoding succeeds"
        );

        if (result.Ok())
        {
            std::string reconstructed;

            for (const auto& span :
                 result.Value())
            {
                reconstructed +=
                    Decode(
                        span,
                        vocabulary
                    );
            }

            Expect(
                reconstructed == input,
                "Mixed Unicode round trip"
            );
        }
    }

    {
        Vocabulary vocabulary;

        const std::string input =
            "தமிழ்";

        const auto result =
            BpeSpanEncoder::Encode(
                input,
                vocabulary
            );

        Expect(
            result.Ok(),
            "Tamil encoding succeeds"
        );

        if (result.Ok())
        {
            std::string reconstructed;

            for (const auto& span :
                 result.Value())
            {
                reconstructed +=
                    Decode(
                        span,
                        vocabulary
                    );
            }

            Expect(
                reconstructed == input,
                "Tamil graphemes preserved"
            );
        }
    }

    {
        Vocabulary vocabulary;

        const std::string invalid{
            static_cast<char>(0xFF)
        };

        const auto result =
            BpeSpanEncoder::Encode(
                invalid,
                vocabulary
            );

        Expect(
            result.Failed(),
            "Invalid UTF-8 rejected"
        );
    }

    const usize failed =
        tests_run - tests_passed;

    std::cout
        << "\n================================\n"
        << "Tests Run    : "
        << tests_run
        << '\n'
        << "Tests Passed : "
        << tests_passed
        << '\n'
        << "Tests Failed : "
        << failed
        << '\n'
        << "================================\n";

    return failed == 0 ? 0 : 1;
}
