#include <string_view>

#include "rules/currency_rule.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::rules;
using namespace qualix::test;

namespace
{

void ExpectCurrency(
    std::string_view input,
    std::string_view expected,
    std::string_view name
)
{
    CurrencyRule rule;

    const auto match =
        rule.Match(input, 0);

    Expect(
        match.Matched(),
        name
    );

    if (!match.Matched())
        return;

    Expect(
        match.type ==
            RuleType::Currency,
        "Currency match type"
    );

    Expect(
        match.View(input) ==
            expected,
        "Currency match text"
    );
}

void ExpectNoCurrency(
    std::string_view input,
    std::string_view name
)
{
    CurrencyRule rule;

    Expect(
        !rule.Match(input, 0).Matched(),
        name
    );
}

} // namespace

int main()
{
    ExpectCurrency(
        "$500",
        "$500",
        "Dollar symbol"
    );

    ExpectCurrency(
        "€500",
        "€500",
        "Euro symbol"
    );

    ExpectCurrency(
        "£500",
        "£500",
        "Pound symbol"
    );

    ExpectCurrency(
        "¥500",
        "¥500",
        "Yen symbol"
    );

    ExpectCurrency(
        "₹500",
        "₹500",
        "Rupee symbol"
    );

    ExpectCurrency(
        "₩500",
        "₩500",
        "Won symbol"
    );

    ExpectCurrency(
        "₽500",
        "₽500",
        "Ruble symbol"
    );

    ExpectCurrency(
        "Rs.500",
        "Rs.500",
        "Indian Rs dot"
    );

    ExpectCurrency(
        "Rs 500",
        "Rs 500",
        "Indian Rs space"
    );

    ExpectCurrency(
        "ரூ.500",
        "ரூ.500",
        "Tamil rupee dot"
    );

    ExpectCurrency(
        "ரூ 500",
        "ரூ 500",
        "Tamil rupee space"
    );

    ExpectCurrency(
        "USD 500",
        "USD 500",
        "USD prefix"
    );

    ExpectCurrency(
        "EUR 500",
        "EUR 500",
        "EUR prefix"
    );

    ExpectCurrency(
        "GBP 500",
        "GBP 500",
        "GBP prefix"
    );

    ExpectCurrency(
        "INR 500",
        "INR 500",
        "INR prefix"
    );

    ExpectCurrency(
        "JPY 500",
        "JPY 500",
        "JPY prefix"
    );

    ExpectCurrency(
        "500 USD",
        "500 USD",
        "USD suffix"
    );

    ExpectCurrency(
        "500 INR",
        "500 INR",
        "INR suffix"
    );

    ExpectCurrency(
        "$1,000",
        "$1,000",
        "Grouped dollar"
    );

    ExpectCurrency(
        "₹1,000",
        "₹1,000",
        "Grouped rupee"
    );

    ExpectCurrency(
        "$1,234.56",
        "$1,234.56",
        "Decimal grouped dollar"
    );

    ExpectCurrency(
        "EUR 1,234.56",
        "EUR 1,234.56",
        "ISO grouped decimal"
    );

    ExpectCurrency(
        "$-500",
        "$-500",
        "Signed dollar"
    );

    ExpectCurrency(
        "USD -500",
        "USD -500",
        "Signed ISO amount"
    );

    ExpectCurrency(
        "₹1.5e3",
        "₹1.5e3",
        "Scientific currency"
    );

    ExpectCurrency(
        "$0xFF",
        "$0xFF",
        "Hex currency number reuse"
    );

    ExpectCurrency(
        "$500.",
        "$500.",
        "Trailing dot decimal currency"
    );

    ExpectCurrency(
        "USD 500,",
        "USD 500",
        "ISO trailing punctuation excluded"
    );

    ExpectNoCurrency(
        "$",
        "Currency symbol without number rejected"
    );

    ExpectNoCurrency(
        "₹",
        "Rupee symbol without number rejected"
    );

    ExpectNoCurrency(
        "Rs.",
        "Rs without number rejected"
    );

    ExpectNoCurrency(
        "USD",
        "ISO code without number rejected"
    );

    ExpectNoCurrency(
        "USD abc",
        "ISO code with text rejected"
    );

    ExpectNoCurrency(
        "abc USD",
        "Text before ISO rejected"
    );

    ExpectNoCurrency(
        "USD500abc",
        "Currency inside identifier rejected"
    );

    ExpectNoCurrency(
        "500 USDabc",
        "Suffix code continuation rejected"
    );

    Expect(
        ToString(
            CurrencyRule{}.Type()
        ) == "Currency",
        "Currency rule reports type"
    );

    return Summary();
}
