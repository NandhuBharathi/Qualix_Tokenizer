#include <memory>
#include <string_view>

#include "rules/rule_engine.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::rules;
using namespace qualix::test;

namespace
{

class LiteralRule final : public Rule
{
public:
    LiteralRule(
        RuleType type,
        std::string_view literal
    )
        : type_(type),
          literal_(literal)
    {
    }

    RuleType Type() const noexcept override
    {
        return type_;
    }

    RuleMatch Match(
        std::string_view input,
        usize byte_offset
    ) const noexcept override
    {
        if (byte_offset > input.size())
            return {};

        if (literal_.size() >
            input.size() - byte_offset)
        {
            return {};
        }

        if (input.substr(
                byte_offset,
                literal_.size()
            ) != literal_)
        {
            return {};
        }

        return RuleMatch{
            type_,
            byte_offset,
            literal_.size()
        };
    }

private:
    RuleType type_;
    std::string_view literal_;
};

} // namespace

int main()
{
    {
        RuleMatch match{};

        Expect(
            match.type == RuleType::None,
            "Default match type"
        );

        Expect(
            match.Empty(),
            "Default match empty"
        );

        Expect(
            !match.Matched(),
            "Default match not matched"
        );
    }

    {
        const std::string_view text =
            "hello@example.com";

        RuleMatch match{
            RuleType::Email,
            0,
            text.size()
        };

        Expect(
            match.Matched(),
            "Rule match matched"
        );

        Expect(
            match.ByteEnd() == text.size(),
            "Rule match byte end"
        );

        Expect(
            match.View(text) == text,
            "Rule match view"
        );
    }

    {
        RuleEngine engine;

        Expect(
            engine.Empty(),
            "Default engine empty"
        );

        Expect(
            engine.RuleCount() == 0,
            "Default engine zero rules"
        );

        Expect(
            !engine.MatchAt(
                "hello",
                0
            ).Matched(),
            "Empty engine no match"
        );
    }

    {
        RuleEngine engine;

        engine.Add(
            std::make_unique<LiteralRule>(
                RuleType::Number,
                "123"
            )
        );

        Expect(
            !engine.Empty(),
            "Engine not empty"
        );

        Expect(
            engine.RuleCount() == 1,
            "Engine one rule"
        );

        auto match =
            engine.MatchAt(
                "x123y",
                1
            );

        Expect(
            match.Matched(),
            "Literal matched"
        );

        Expect(
            match.type ==
                RuleType::Number,
            "Literal match type"
        );

        Expect(
            match.View("x123y") ==
                "123",
            "Literal match view"
        );
    }

    {
        RuleEngine engine;

        engine.Add(
            std::make_unique<LiteralRule>(
                RuleType::Number,
                "12"
            )
        );

        engine.Add(
            std::make_unique<LiteralRule>(
                RuleType::Number,
                "12345"
            )
        );

        engine.Add(
            std::make_unique<LiteralRule>(
                RuleType::Number,
                "123"
            )
        );

        auto match =
            engine.MatchAt(
                "12345!",
                0
            );

        Expect(
            match.Matched(),
            "Longest match exists"
        );

        Expect(
            match.byte_length == 5,
            "Longest match selected"
        );

        Expect(
            match.View("12345!") ==
                "12345",
            "Longest match text"
        );
    }

    {
        RuleEngine engine;

        engine.Add(
            std::make_unique<LiteralRule>(
                RuleType::Email,
                "abc"
            )
        );

        auto match =
            engine.MatchAt(
                "xabc",
                0
            );

        Expect(
            !match.Matched(),
            "Match anchored at offset"
        );
    }

    {
        RuleEngine engine;

        engine.Add(nullptr);

        Expect(
            engine.RuleCount() == 0,
            "Null rule ignored"
        );
    }

    Expect(
        ToString(RuleType::Url) ==
            "Url",
        "URL rule type string"
    );

    Expect(
        ToString(RuleType::Email) ==
            "Email",
        "Email rule type string"
    );

    Expect(
        ToString(RuleType::Currency) ==
            "Currency",
        "Currency rule type string"
    );

    return Summary();
}
