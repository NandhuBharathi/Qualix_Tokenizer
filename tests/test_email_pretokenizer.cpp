#include <string>

#include "pretokenizer/pretokenizer.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::pretokenizer;
using namespace qualix::test;

int main()
{
    {
        const std::string text =
            "Contact user@example.com today";

        auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "Email sentence split"
        );

        if (result.Ok())
        {
            const auto& spans =
                result.Value();

            Expect(
                spans.size() == 5,
                "Email sentence span count"
            );

            if (spans.size() == 5)
            {
                Expect(
                    spans[0].View(text) ==
                        "Contact",
                    "Word before email"
                );

                Expect(
                    spans[1].type ==
                        SpanType::Whitespace,
                    "Whitespace before email"
                );

                Expect(
                    spans[2].View(text) ==
                        "user@example.com",
                    "Full email preserved"
                );

                Expect(
                    spans[2].type ==
                        SpanType::Email,
                    "Email span type"
                );

                Expect(
                    spans[2].Protected(),
                    "Email protected"
                );

                Expect(
                    spans[3].type ==
                        SpanType::Whitespace,
                    "Whitespace after email"
                );

                Expect(
                    spans[4].View(text) ==
                        "today",
                    "Word after email"
                );
            }
        }
    }

    {
        const std::string text =
            "Mail first.last+tag@sub.example.co.in.";

        auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "Complex email sentence"
        );

        if (result.Ok())
        {
            const auto& spans =
                result.Value();

            Expect(
                spans.size() == 4,
                "Complex email span count"
            );

            if (spans.size() == 4)
            {
                Expect(
                    spans[2].View(text) ==
                        "first.last+tag@sub.example.co.in",
                    "Complex email preserved"
                );

                Expect(
                    spans[2].type ==
                        SpanType::Email,
                    "Complex email type"
                );

                Expect(
                    spans[2].Protected(),
                    "Complex email protected"
                );

                Expect(
                    spans[3].View(text) == ".",
                    "Email trailing period separate"
                );

                Expect(
                    spans[3].type ==
                        SpanType::Punctuation,
                    "Email period punctuation"
                );
            }
        }
    }

    {
        const std::string text =
            "தமிழ்@example.com";

        auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "Unicode local email split"
        );

        if (result.Ok())
        {
            const auto& spans =
                result.Value();

            Expect(
                spans.size() == 1,
                "Unicode local email one span"
            );

            if (spans.size() == 1)
            {
                Expect(
                    spans[0].View(text) == text,
                    "Unicode local email preserved"
                );

                Expect(
                    spans[0].type ==
                        SpanType::Email,
                    "Unicode local email type"
                );

                Expect(
                    spans[0].Protected(),
                    "Unicode local email protected"
                );
            }
        }
    }

    {
        const std::string text =
            "user@例子.测试";

        auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "Unicode domain email split"
        );

        if (result.Ok())
        {
            const auto& spans =
                result.Value();

            Expect(
                spans.size() == 1,
                "Unicode domain email one span"
            );

            if (spans.size() == 1)
            {
                Expect(
                    spans[0].View(text) == text,
                    "Unicode domain email preserved"
                );

                Expect(
                    spans[0].type ==
                        SpanType::Email,
                    "Unicode domain email type"
                );

                Expect(
                    spans[0].Protected(),
                    "Unicode domain email protected"
                );
            }
        }
    }

    {
        const std::string text =
            "user@example.com https://example.com";

        auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "Email and URL coexist"
        );

        if (result.Ok())
        {
            const auto& spans =
                result.Value();

            Expect(
                spans.size() == 3,
                "Email URL span count"
            );

            if (spans.size() == 3)
            {
                Expect(
                    spans[0].type ==
                        SpanType::Email,
                    "First span email"
                );

                Expect(
                    spans[0].View(text) ==
                        "user@example.com",
                    "Email not classified as URL"
                );

                Expect(
                    spans[0].Protected(),
                    "Email coexist protected"
                );

                Expect(
                    spans[1].type ==
                        SpanType::Whitespace,
                    "Email URL whitespace"
                );

                Expect(
                    spans[2].type ==
                        SpanType::Url,
                    "Second span URL"
                );

                Expect(
                    spans[2].View(text) ==
                        "https://example.com",
                    "URL preserved beside email"
                );

                Expect(
                    spans[2].Protected(),
                    "URL coexist protected"
                );
            }
        }
    }

    {
        const std::string text =
            "user@example";

        auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "Invalid email normal split"
        );

        if (result.Ok())
        {
            bool has_email = false;

            for (const auto& span :
                 result.Value())
            {
                if (span.type ==
                    SpanType::Email)
                {
                    has_email = true;
                }
            }

            Expect(
                !has_email,
                "Invalid email not protected"
            );
        }
    }

    {
        const std::string text =
            "Hello தமிழ் 123 👍🏽";

        auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "Normal pipeline preserved"
        );

        if (result.Ok())
        {
            bool number_protected = false;
            bool unexpected_protected = false;

            for (const auto& span :
                 result.Value())
            {
                if (!span.Protected())
                    continue;

                const auto view =
                    span.View(text);

                if (view == "123" &&
                    span.type ==
                        SpanType::Number)
                {
                    number_protected = true;
                }
                else
                {
                    unexpected_protected = true;
                }
            }

            Expect(
                number_protected,
                "Number rule preserved"
            );

            Expect(
                !unexpected_protected,
                "Only recognized number protected"
            );
        }
    }

    return Summary();
}
