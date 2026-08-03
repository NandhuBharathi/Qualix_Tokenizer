#include "rules/number_rule.hpp"
#include "rules/numeric_scanner.hpp"

namespace qualix::rules
{

RuleMatch NumberRule::Match(
    std::string_view input,
    usize offset
) const noexcept
{
    const NumericScan scan=
        NumericScanner::Scan(
            input,
            offset
        );

    if (!scan.matched)
        return {};

    return RuleMatch{
        RuleType::Number,
        scan.start,
        scan.ByteLength()
    };
}

} // namespace qualix::rules
