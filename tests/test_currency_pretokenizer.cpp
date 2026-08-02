#include <string>
#include <string_view>

#include "pretokenizer/pretokenizer.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::pretokenizer;
using namespace qualix::test;

namespace
{

void ExpectCurrency(
    std::string_view input,
    std::string_view expected,
    std::string_view name
)
{
    auto result =
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
        "Currency one span"
    );

    if (spans.size() != 1)
        return;

    Expect(
        spans[0].View(input) == expected,
        "Currency text preserved"
    );

    Expect(
        spans[0].type ==
            SpanType::Currency,
        "Currency span type"
    );

    Expect(
        spans[0].Protected(),
        "Currency protected"
    );
}

} // namespace

int main()
{
    ExpectCurrency(
        "$500",
        "$500",
        "Dollar currency protected"
    );

    ExpectCurrency(
        "€500",
        "€500",
        "Euro currency protected"
    );

    ExpectCurrency(
        "£500",
        "£500",
        "Pound currency protected"
    );

    ExpectCurrency(
        "¥500",
        "¥500",
        "Yen currency protected"
    );

    ExpectCurrency(
        "₹500",
        "₹500",
        "Rupee currency protected"
    );

    ExpectCurrency(
        "₩500",
        "₩500",
        "Won currency protected"
    );

    ExpectCurrency(
        "₽500",
        "₽500",
        "Ruble currency protected"
    );

    ExpectCurrency(
        "Rs.500",
        "Rs.500",
        "Indian Rs dot protected"
    );

    ExpectCurrency(
        "Rs 500",
        "Rs 500",
        "Indian Rs space protected"
    );

    ExpectCurrency(
        "ரூ.500",
        "ரூ.500",
        "Tamil rupee dot protected"
    );

    ExpectCurrency(
        "ரூ 500",
        "ரூ 500",
        "Tamil rupee space protected"
    );

    ExpectCurrency(
        "USD 500",
        "USD 500",
        "USD prefix protected"
    );

    ExpectCurrency(
        "EUR 1,000.50",
        "EUR 1,000.50",
        "ISO grouped decimal protected"
    );

    ExpectCurrency(
        "500 USD",
        "500 USD",
        "USD suffix protected"
    );

    ExpectCurrency(
        "1,000 INR",
        "1,000 INR",
        "INR suffix protected"
    );

    ExpectCurrency(
        "₹1,25,000",
        "₹1,25,000",
        "Indian grouped rupee protected"
    );

    ExpectCurrency(
        "$1,000.50",
        "$1,000.50",
        "Grouped decimal dollar protected"
    );

    ExpectCurrency(
        "$1.2e10",
        "$1.2e10",
        "Scientific currency protected"
    );

    ExpectCurrency(
        "$0xFF",
        "$0xFF",
        "Hex currency protected"
    );

    {
        const std::string text =
            "Price is ₹1,25,000 today";

        auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "Currency sentence split"
        );

        if (result.Ok())
        {
            const auto& spans =
                result.Value();

            bool found = false;

            for (const auto& span : spans)
            {
                if (span.type ==
                        SpanType::Currency &&
                    span.View(text) ==
                        "₹1,25,000")
                {
                    found =
                        span.Protected();
                }
            }

            Expect(
                found,
                "Sentence currency protected"
            );
        }
    }

    {
        const std::string text =
            "Pay $500, now";

        auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "Currency punctuation split"
        );

        if (result.Ok())
        {
            const auto& spans =
                result.Value();

            bool currency_found = false;
            bool comma_separate = false;

            for (const auto& span : spans)
            {
                if (span.type ==
                        SpanType::Currency &&
                    span.View(text) ==
                        "$500")
                {
                    currency_found =
                        span.Protected();
                }

                if (span.type ==
                        SpanType::Punctuation &&
                    span.View(text) == ",")
                {
                    comma_separate = true;
                }
            }

            Expect(
                currency_found,
                "Currency before punctuation"
            );

            Expect(
                comma_separate,
                "Currency punctuation separate"
            );
        }
    }

    {
        const std::string text =
            "50% ₹500 user@example.com https://example.com 123";

        auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "All rules coexist"
        );

        if (result.Ok())
        {
            bool percentage = false;
            bool currency = false;
            bool email = false;
            bool url = false;
            bool number = false;

            for (const auto& span :
                 result.Value())
            {
                const auto view =
                    span.View(text);

                if (span.type ==
                        SpanType::Percentage &&
                    view == "50%")
                {
                    percentage =
                        span.Protected();
                }

                if (span.type ==
                        SpanType::Currency &&
                    view == "₹500")
                {
                    currency =
                        span.Protected();
                }

                if (span.type ==
                        SpanType::Email &&
                    view ==
                        "user@example.com")
                {
                    email =
                        span.Protected();
                }

                if (span.type ==
                        SpanType::Url &&
                    view ==
                        "https://example.com")
                {
                    url =
                        span.Protected();
                }

                if (span.type ==
                        SpanType::Number &&
                    view == "123")
                {
                    number =
                        span.Protected();
                }
            }

            Expect(
                percentage,
                "Percentage coexist"
            );

            Expect(
                currency,
                "Currency coexist"
            );

            Expect(
                email,
                "Email coexist"
            );

            Expect(
                url,
                "URL coexist"
            );

            Expect(
                number,
                "Number coexist"
            );
        }
    }

    return Summary();
}
