#include <string_view>

#include "rules/url_rule.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::rules;
using namespace qualix::test;

namespace
{

void ExpectUrl(
    std::string_view input,
    std::string_view expected,
    std::string_view name
)
{
    UrlRule rule;

    const auto match =
        rule.Match(input, 0);

    Expect(
        match.Matched(),
        name
    );

    if (match.Matched())
    {
        Expect(
            match.type == RuleType::Url,
            "URL match type"
        );

        Expect(
            match.View(input) == expected,
            "URL match text"
        );
    }
}

void ExpectNoUrl(
    std::string_view input,
    std::string_view name
)
{
    UrlRule rule;

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
    ExpectUrl(
        "https://example.com",
        "https://example.com",
        "HTTPS URL"
    );

    ExpectUrl(
        "http://example.com/path",
        "http://example.com/path",
        "HTTP URL path"
    );

    ExpectUrl(
        "https://example.com/a?x=1&y=2",
        "https://example.com/a?x=1&y=2",
        "URL query"
    );

    ExpectUrl(
        "https://example.com/page#section",
        "https://example.com/page#section",
        "URL fragment"
    );

    ExpectUrl(
        "www.example.com",
        "www.example.com",
        "WWW URL"
    );

    ExpectUrl(
        "example.com",
        "example.com",
        "Bare domain"
    );

    ExpectUrl(
        "sub.example.co.in",
        "sub.example.co.in",
        "Subdomain"
    );

    ExpectUrl(
        "https://example.com/தமிழ்",
        "https://example.com/தமிழ்",
        "Unicode URL path"
    );

    ExpectUrl(
        "https://例子.测试/",
        "https://例子.测试/",
        "Unicode domain"
    );

    ExpectUrl(
        "https://example.com.",
        "https://example.com",
        "Trailing period trimmed"
    );

    ExpectUrl(
        "https://example.com,",
        "https://example.com",
        "Trailing comma trimmed"
    );

    ExpectUrl(
        "https://example.com!",
        "https://example.com",
        "Trailing exclamation trimmed"
    );

    ExpectUrl(
        "https://example.com/wiki/Test_(example)",
        "https://example.com/wiki/Test_(example)",
        "Balanced parentheses preserved"
    );

    ExpectUrl(
        "https://example.com/test)",
        "https://example.com/test",
        "Unmatched parenthesis excluded"
    );

    ExpectNoUrl(
        "example",
        "Plain word rejected"
    );

    ExpectNoUrl(
        "localhost",
        "Localhost rejected"
    );

    ExpectNoUrl(
        ".example.com",
        "Leading dot rejected"
    );

    ExpectNoUrl(
        "example..com",
        "Empty domain label rejected"
    );

    ExpectNoUrl(
        "-example.com",
        "Leading hyphen rejected"
    );

    ExpectNoUrl(
        "example-.com",
        "Trailing label hyphen rejected"
    );

    ExpectNoUrl(
        "user@example.com",
        "Email not URL"
    );

    ExpectNoUrl(
        "12.75",
        "Decimal not URL"
    );

    ExpectNoUrl(
        "1.2e10",
        "Scientific number not URL"
    );

    ExpectNoUrl(
        "192.168",
        "Numeric dotted value not URL"
    );

    Expect(
        ToString(
            UrlRule{}.Type()
        ) == "Url",
        "URL rule reports type"
    );

    return Summary();
}
