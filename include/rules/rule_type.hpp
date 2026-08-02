#pragma once

#include <string_view>

namespace qualix::rules
{

enum class RuleType
{
    None,
    Url,
    Email,
    Date,
    Time,
    Number,
    Currency,
    Percentage,
    Measurement,
    Math,
    Code
};

[[nodiscard]]
constexpr std::string_view ToString(
    RuleType type
) noexcept
{
    switch (type)
    {
        case RuleType::None:
            return "None";
        case RuleType::Url:
            return "Url";
        case RuleType::Email:
            return "Email";
        case RuleType::Date:
            return "Date";
        case RuleType::Time:
            return "Time";
        case RuleType::Number:
            return "Number";
        case RuleType::Currency:
            return "Currency";
        case RuleType::Percentage:
            return "Percentage";
        case RuleType::Measurement:
            return "Measurement";
        case RuleType::Math:
            return "Math";
        case RuleType::Code:
            return "Code";
    }

    return "Unknown";
}

} // namespace qualix::rules
