#include <string>
#include <string_view>

#include "pretokenizer/pretokenizer.hpp"
#include "pretokenizer/span_type.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::pretokenizer;
using namespace qualix::test;

namespace
{

void ExpectProtectedPercentage(
    std::string_view input,
    std::string_view name
)
{
    const auto result =
        PreTokenizer::Split(input);

    Expect(
        result.Ok(),
        name
    );

    if (!result.Ok())
        return;

    const auto& spans =
        result.Value();

    Expect(
        spans.size() == 1,
        "Percentage one span"
    );

    if (spans.size() != 1)
        return;

    Expect(
        spans[0].View(input) == input,
        "Percentage text preserved"
    );

    Expect(
        spans[0].type ==
            SpanType::Percentage,
        "Percentage span type"
    );

    Expect(
        spans[0].Protected(),
        "Percentage protected"
    );
}

} // namespace

int main()
{
    ExpectProtectedPercentage(
        "50%",
        "Integer percentage protected"
    );

    ExpectProtectedPercentage(
        "12.5%",
        "Decimal percentage protected"
    );

    ExpectProtectedPercentage(
        "+25%",
        "Positive percentage protected"
    );

    ExpectProtectedPercentage(
        "-25%",
        "Negative percentage protected"
    );

    ExpectProtectedPercentage(
        "1,000%",
        "Grouped percentage protected"
    );

    ExpectProtectedPercentage(
        "1.5e2%",
        "Scientific percentage protected"
    );

    ExpectProtectedPercentage(
        "50‰",
        "Per mille protected"
    );

    ExpectProtectedPercentage(
        "12.5‰",
        "Decimal per mille protected"
    );

    {
        const std::string text =
            "Rate is 12.5% today";

        const auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "Percentage sentence split"
        );

        if (result.Ok())
        {
            const auto& spans =
                result.Value();

            bool found = false;

            for (const auto& span : spans)
            {
                if (span.View(text) ==
                    "12.5%")
                {
                    found = true;

                    Expect(
                        span.type ==
                            SpanType::Percentage,
                        "Sentence percentage type"
                    );

                    Expect(
                        span.Protected(),
                        "Sentence percentage protected"
                    );
                }
            }

            Expect(
                found,
                "Sentence percentage preserved"
            );
        }
    }

    {
        const std::string text =
            "50%.";

        const auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "Percentage punctuation split"
        );

        if (result.Ok())
        {
            const auto& spans =
                result.Value();

            Expect(
                spans.size() == 2,
                "Percentage punctuation span count"
            );

            if (spans.size() == 2)
            {
                Expect(
                    spans[0].View(text) ==
                        "50%",
                    "Percentage before punctuation"
                );

                Expect(
                    spans[0].Protected(),
                    "Percentage before punctuation protected"
                );

                Expect(
                    spans[1].View(text) ==
                        ".",
                    "Percentage punctuation separate"
                );

                Expect(
                    spans[1].type ==
                        SpanType::Punctuation,
                    "Percentage punctuation type"
                );
            }
        }
    }

    {
        const std::string text =
            "5! 50% user@example.com https://example.com";

        const auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "Rules coexist"
        );

        if (result.Ok())
        {
            bool number_found = false;
            bool percentage_found = false;
            bool email_found = false;
            bool url_found = false;

            for (const auto& span :
                 result.Value())
            {
                const auto view =
                    span.View(text);

                if (view == "5")
                {
                    number_found =
                        span.Protected() &&
                        span.type ==
                            SpanType::Number;
                }
                else if (view == "50%")
                {
                    percentage_found =
                        span.Protected() &&
                        span.type ==
                            SpanType::Percentage;
                }
                else if (
                    view ==
                    "user@example.com")
                {
                    email_found =
                        span.Protected() &&
                        span.type ==
                            SpanType::Email;
                }
                else if (
                    view ==
                    "https://example.com")
                {
                    url_found =
                        span.Protected() &&
                        span.type ==
                            SpanType::Url;
                }
            }

            Expect(
                number_found,
                "Number coexist"
            );

            Expect(
                percentage_found,
                "Percentage coexist"
            );

            Expect(
                email_found,
                "Email coexist"
            );

            Expect(
                url_found,
                "URL coexist"
            );
        }
    }

    return Summary();
}
