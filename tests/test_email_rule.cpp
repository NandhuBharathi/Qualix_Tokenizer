#include <string_view>

#include "rules/email_rule.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::rules;
using namespace qualix::test;

namespace
{

void ExpectEmail(
    std::string_view input,
    std::string_view expected,
    std::string_view name
)
{
    EmailRule rule;

    const auto match =
        rule.Match(input, 0);

    Expect(
        match.Matched(),
        name
    );

    if (match.Matched())
    {
        Expect(
            match.type ==
                RuleType::Email,
            "Email match type"
        );

        Expect(
            match.View(input) ==
                expected,
            "Email match text"
        );
    }
}

void ExpectNoEmail(
    std::string_view input,
    std::string_view name
)
{
    EmailRule rule;

    Expect(
        !rule.Match(
            input,
            0
        ).Matched(),
        name
    );
}

} // namespace

int main()
{
    ExpectEmail(
        "user@example.com",
        "user@example.com",
        "Basic email"
    );

    ExpectEmail(
        "first.last@example.com",
        "first.last@example.com",
        "Dotted local part"
    );

    ExpectEmail(
        "name+tag@example.com",
        "name+tag@example.com",
        "Plus tag"
    );

    ExpectEmail(
        "user_123@sub.example.org",
        "user_123@sub.example.org",
        "Subdomain email"
    );

    ExpectEmail(
        "user-name@example.co.in",
        "user-name@example.co.in",
        "Hyphen local part"
    );

    ExpectEmail(
        "user@example.com.",
        "user@example.com",
        "Trailing period excluded"
    );

    ExpectEmail(
        "user@example.com,",
        "user@example.com",
        "Trailing comma excluded"
    );

    ExpectEmail(
        "user@例子.测试",
        "user@例子.测试",
        "Unicode domain"
    );

    ExpectEmail(
        "தமிழ்@example.com",
        "தமிழ்@example.com",
        "Unicode local part"
    );

    ExpectNoEmail(
        "userexample.com",
        "Missing at sign"
    );

    ExpectNoEmail(
        "@example.com",
        "Missing local part"
    );

    ExpectNoEmail(
        "user@",
        "Missing domain"
    );

    ExpectNoEmail(
        "user@example",
        "Domain without dot"
    );

    ExpectNoEmail(
        ".user@example.com",
        "Leading local dot"
    );

    ExpectNoEmail(
        "user..name@example.com",
        "Double local dot"
    );

    ExpectNoEmail(
        "user.@example.com",
        "Trailing local dot"
    );

    ExpectNoEmail(
        "user@.example.com",
        "Leading domain dot"
    );

    ExpectNoEmail(
        "user@example..com",
        "Double domain dot"
    );

    ExpectNoEmail(
        "user@-example.com",
        "Leading domain hyphen"
    );

    ExpectNoEmail(
        "user@example-.com",
        "Trailing domain hyphen"
    );

    Expect(
        ToString(
            EmailRule{}.Type()
        ) == "Email",
        "Email rule reports type"
    );

    return Summary();
}
