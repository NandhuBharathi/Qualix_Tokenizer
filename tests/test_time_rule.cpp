#include <string_view>

#include "rules/time_rule.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::rules;
using namespace qualix::test;

namespace
{

void ExpectMatch(
    const TimeRule& rule,
    std::string_view input,
    std::string_view expected,
    const char* name
)
{
    const auto match = rule.Match(input, 0);

    Expect(
        match.Matched(),
        name
    );

    if (match.Matched())
    {
        Expect(
            match.type == RuleType::Time,
            "Time match type"
        );

        Expect(
            input.substr(
                match.byte_start,
                match.ByteEnd() - match.byte_start
            ) == expected,
            "Time match text"
        );
    }
}

void ExpectNoMatch(
    const TimeRule& rule,
    std::string_view input,
    const char* name
)
{
    Expect(
        !rule.Match(input, 0).Matched(),
        name
    );
}

} // namespace

int main()
{
    TimeRule rule;

    ExpectMatch(
        rule,
        "10:30",
        "10:30",
        "Basic time"
    );

    ExpectMatch(
        rule,
        "23:59",
        "23:59",
        "24 hour time"
    );

    ExpectMatch(
        rule,
        "09:05",
        "09:05",
        "Leading zero time"
    );

    ExpectMatch(
        rule,
        "10:30:45",
        "10:30:45",
        "Time with seconds"
    );

    ExpectMatch(
        rule,
        "9 AM",
        "9 AM",
        "Hour AM"
    );

    ExpectMatch(
        rule,
        "9 PM",
        "9 PM",
        "Hour PM"
    );

    ExpectMatch(
        rule,
        "9am",
        "9am",
        "Compact AM"
    );

    ExpectMatch(
        rule,
        "9pm",
        "9pm",
        "Compact PM"
    );

    ExpectMatch(
        rule,
        "10:30 AM",
        "10:30 AM",
        "Time with AM"
    );

    ExpectMatch(
        rule,
        "10:30PM",
        "10:30PM",
        "Compact time with PM"
    );

    ExpectMatch(
        rule,
        "12:00 AM",
        "12:00 AM",
        "Midnight"
    );

    ExpectMatch(
        rule,
        "12:00 PM",
        "12:00 PM",
        "Noon"
    );

    ExpectNoMatch(
        rule,
        "25:00",
        "Invalid hour rejected"
    );

    ExpectNoMatch(
        rule,
        "12:60",
        "Invalid minute rejected"
    );

    ExpectNoMatch(
        rule,
        "10:30:60",
        "Invalid second rejected"
    );

    ExpectNoMatch(
        rule,
        "13 PM",
        "Invalid 12 hour time rejected"
    );

    ExpectNoMatch(
        rule,
        "0 AM",
        "Zero AM hour rejected"
    );

    ExpectNoMatch(
        rule,
        "123",
        "Bare number rejected"
    );

    ExpectNoMatch(
        rule,
        "time",
        "Word rejected"
    );

    Expect(
        rule.Type() == RuleType::Time,
        "Time rule reports type"
    );

    return Summary();
}
