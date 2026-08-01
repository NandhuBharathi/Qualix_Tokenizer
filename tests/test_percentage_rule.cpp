#include <string>
#include <string_view>

#include "rules/percentage_rule.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::rules;
using namespace qualix::test;

namespace
{

void ExpectPercentage(
    std::string_view input,
    std::string_view expected,
    std::string_view name
)
{
    PercentageRule rule;

    const RuleMatch match =
        rule.Match(input, 0);

    Expect(
        match.Matched(),
        name
    );

    if (!match.Matched())
        return;

    Expect(
        match.type ==
            RuleType::Percentage,
        "Percentage match type"
    );

    Expect(
        match.View(input) ==
            expected,
        "Percentage match text"
    );
}

void ExpectNoPercentage(
    std::string_view input,
    std::string_view name
)
{
    PercentageRule rule;

    const RuleMatch match =
        rule.Match(input, 0);

    Expect(
        !match.Matched(),
        name
    );
}

} // namespace

int main()
{
    ExpectPercentage(
        "50%",
        "50%",
        "Integer percentage"
    );

    ExpectPercentage(
        "12.5%",
        "12.5%",
        "Decimal percentage"
    );

    ExpectPercentage(
        "+25%",
        "+25%",
        "Positive percentage"
    );

    ExpectPercentage(
        "-25%",
        "-25%",
        "Negative percentage"
    );

    ExpectPercentage(
        "1,000%",
        "1,000%",
        "Grouped percentage"
    );

    ExpectPercentage(
        "1.5e2%",
        "1.5e2%",
        "Scientific percentage"
    );

    ExpectPercentage(
        "0x10%",
        "0x10%",
        "Hexadecimal percentage"
    );

    ExpectPercentage(
        "0b1010%",
        "0b1010%",
        "Binary percentage"
    );

    ExpectPercentage(
        "0o17%",
        "0o17%",
        "Octal percentage"
    );

    ExpectPercentage(
        "50‰",
        "50‰",
        "Per mille"
    );

    ExpectPercentage(
        "12.5‰",
        "12.5‰",
        "Decimal per mille"
    );

    {
        const std::string input =
            "50%.";

        PercentageRule rule;

        const RuleMatch match =
            rule.Match(input, 0);

        Expect(
            match.Matched(),
            "Trailing punctuation percentage"
        );

        if (match.Matched())
        {
            Expect(
                match.View(input) ==
                    "50%",
                "Trailing punctuation excluded"
            );
        }
    }

    {
        const std::string input =
            "50% value";

        PercentageRule rule;

        const RuleMatch match =
            rule.Match(input, 0);

        Expect(
            match.Matched(),
            "Percentage before whitespace"
        );

        if (match.Matched())
        {
            Expect(
                match.View(input) ==
                    "50%",
                "Percentage stops before whitespace"
            );
        }
    }

    ExpectNoPercentage(
        "50",
        "Number without percent rejected"
    );

    ExpectNoPercentage(
        "%",
        "Percent only rejected"
    );

    ExpectNoPercentage(
        "‰",
        "Per mille only rejected"
    );

    ExpectNoPercentage(
        "abc%",
        "Text percentage rejected"
    );

    ExpectNoPercentage(
        "50%abc",
        "Percentage inside identifier rejected"
    );

    ExpectNoPercentage(
        "50%123",
        "Percentage followed by number rejected"
    );

    ExpectNoPercentage(
        "50%தமிழ்",
        "Percentage followed by Unicode rejected"
    );

    Expect(
        ToString(
            PercentageRule{}.Type()
        ) == "Percentage",
        "Percentage rule reports type"
    );

    return Summary();
}
