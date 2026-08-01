#include <string>
#include <string_view>
#include <vector>

#include "pretokenizer/pretokenizer.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::pretokenizer;
using namespace qualix::test;

int main()
{
    {
        const std::string text =
            "Visit https://example.com/path?q=1 today";

        auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "URL sentence split"
        );

        if (result.Ok())
        {
            const auto& spans =
                result.Value();

            Expect(
                spans.size() == 5,
                "URL sentence span count"
            );

            if (spans.size() == 5)
            {
                Expect(
                    spans[0].View(text) == "Visit",
                    "Word before URL"
                );

                Expect(
                    spans[1].type ==
                        SpanType::Whitespace,
                    "Whitespace before URL"
                );

                Expect(
                    spans[2].View(text) ==
                        "https://example.com/path?q=1",
                    "Full URL preserved"
                );

                Expect(
                    spans[2].type ==
                        SpanType::Url,
                    "URL span type"
                );

                Expect(
                    spans[2].Protected(),
                    "URL protected"
                );

                Expect(
                    spans[3].type ==
                        SpanType::Whitespace,
                    "Whitespace after URL"
                );

                Expect(
                    spans[4].View(text) == "today",
                    "Word after URL"
                );
            }
        }
    }

    {
        const std::string text =
            "Open example.com.";

        auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "Bare URL sentence split"
        );

        if (result.Ok())
        {
            const auto& spans =
                result.Value();

            Expect(
                spans.size() == 4,
                "Bare URL punctuation span count"
            );

            if (spans.size() == 4)
            {
                Expect(
                    spans[2].View(text) ==
                        "example.com",
                    "Bare URL protected text"
                );

                Expect(
                    spans[2].type ==
                        SpanType::Url,
                    "Bare URL type"
                );

                Expect(
                    spans[2].Protected(),
                    "Bare URL protected"
                );

                Expect(
                    spans[3].View(text) == ".",
                    "Trailing period separate"
                );

                Expect(
                    spans[3].type ==
                        SpanType::Punctuation,
                    "Trailing period punctuation"
                );
            }
        }
    }

    {
        const std::string text =
            "https://example.com/தமிழ்";

        auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "Unicode path split"
        );

        if (result.Ok())
        {
            const auto& spans =
                result.Value();

            Expect(
                spans.size() == 1,
                "Unicode URL one span"
            );

            if (spans.size() == 1)
            {
                Expect(
                    spans[0].View(text) == text,
                    "Unicode URL preserved"
                );

                Expect(
                    spans[0].type ==
                        SpanType::Url,
                    "Unicode URL type"
                );

                Expect(
                    spans[0].Protected(),
                    "Unicode URL protected"
                );
            }
        }
    }

    {
        const std::string text =
            "https://example.com/wiki/Test_(example)";

        auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "Parenthesized URL split"
        );

        if (result.Ok())
        {
            const auto& spans =
                result.Value();

            Expect(
                spans.size() == 1,
                "Parenthesized URL one span"
            );

            if (spans.size() == 1)
            {
                Expect(
                    spans[0].View(text) == text,
                    "Parenthesized URL preserved"
                );

                Expect(
                    spans[0].Protected(),
                    "Parenthesized URL protected"
                );
            }
        }
    }

    {
        const std::string text =
            "Hello தமிழ் 123 👍🏽";

        auto result =
            PreTokenizer::Split(text);

        Expect(
            result.Ok(),
            "Normal text still splits"
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

    {
        std::string invalid("\x80", 1);

        auto result =
            PreTokenizer::Split(invalid);

        Expect(
            result.Failed(),
            "Invalid UTF8 still rejected"
        );
    }

    return Summary();
}
