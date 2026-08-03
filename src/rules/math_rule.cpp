#include "rules/math_rule.hpp"
#include "rules/numeric_scanner.hpp"

namespace qualix::rules
{

namespace
{


NumericScanPolicy MathNumericPolicy() noexcept
{
    NumericScanPolicy policy{};

    policy.allow_based_numbers=false;
    policy.allow_grouping=false;
    policy.allow_underscore=false;
    policy.allow_leading_dot=false;
    policy.allow_trailing_dot=false;
    policy.allow_sign=false;
    policy.allow_exponent=false;

    return policy;
}


bool IsDigit(char c) noexcept
{
    return c >= '0' && c <= '9';
}

bool IsAlpha(char c) noexcept
{
    return
        (c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z');
}

bool IsIdentifierStart(char c) noexcept
{
    return IsAlpha(c) || c == '_';
}

bool IsIdentifierContinuation(char c) noexcept
{
    return
        IsAlpha(c) ||
        IsDigit(c) ||
        c == '_';
}

bool IsSpace(char c) noexcept
{
    return c == ' ' || c == '\t';
}

bool IsOperator(char c) noexcept
{
    switch (c)
    {
        case '+':
        case '-':
        case '*':
        case '/':
        case '^':
        case '=':
            return true;

        default:
            return false;
    }
}

bool IsBoundaryChar(
    unsigned char c
) noexcept
{
    return
        (c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') ||
        c == '_' ||
        c >= 0x80;
}

void SkipSpaces(
    std::string_view input,
    usize& pos
) noexcept
{
    while (pos < input.size() &&
           IsSpace(input[pos]))
    {
        ++pos;
    }
}


bool ParseIdentifier(
    std::string_view input,
    usize& pos
) noexcept
{
    if (pos >= input.size() ||
        !IsIdentifierStart(input[pos]))
    {
        return false;
    }

    ++pos;

    while (pos < input.size() &&
           IsIdentifierContinuation(
               input[pos]))
    {
        ++pos;
    }

    return true;
}

bool ParseOperand(
    std::string_view input,
    usize& pos,
    int& parentheses
) noexcept
{
    SkipSpaces(input, pos);

    if (pos >= input.size())
        return false;

    if (input[pos] == '(')
    {
        ++parentheses;
        ++pos;
        return true;
    }

    if (IsDigit(input[pos]))
    {
        const NumericScan number =
            NumericScanner::ScanRaw(
                input,
                pos,
                MathNumericPolicy()
            );

        if (!number.matched)
            return false;

        pos = number.end;
        return true;
    }

    if (IsIdentifierStart(input[pos]))
        return ParseIdentifier(input, pos);

    return false;
}

bool ValidBoundary(
    std::string_view input,
    usize begin,
    usize end
) noexcept
{
    if (begin > 0)
    {
        const auto previous =
            static_cast<unsigned char>(
                input[begin - 1]
            );

        if (IsBoundaryChar(previous))
            return false;
    }

    if (end < input.size())
    {
        const auto next =
            static_cast<unsigned char>(
                input[end]
            );

        if (IsBoundaryChar(next))
            return false;
    }

    return true;
}

} // namespace

RuleMatch MathRule::Match(
    std::string_view input,
    usize byte_offset
) const noexcept
{
    if (byte_offset >= input.size())
        return {};

    // A Math span must begin exactly at byte_offset.
    // Spaces are allowed only inside an expression.
    if (IsSpace(input[byte_offset]))
        return {};

    usize pos = byte_offset;

    int parentheses = 0;

    bool expect_operand = true;
    bool has_operator = false;
    bool has_real_operand = false;

    while (pos < input.size())
    {
        SkipSpaces(input, pos);

        if (pos >= input.size())
            break;

        if (expect_operand)
        {
            const usize before = pos;

            if (!ParseOperand(
                    input,
                    pos,
                    parentheses))
            {
                return {};
            }

            // '(' opens a group but is not itself
            // a complete operand.
            if (input[before] == '(')
            {
                expect_operand = true;
                continue;
            }

            has_real_operand = true;
            expect_operand = false;
            continue;
        }

        SkipSpaces(input, pos);

        if (pos >= input.size())
            break;

        if (input[pos] == ')')
        {
            if (parentheses <= 0)
                break;

            --parentheses;
            ++pos;
            expect_operand = false;
            continue;
        }

        if (IsOperator(input[pos]))
        {
            /*
             * Disambiguate prose followed by a signed number:
             *
             *   "Value -12.5 test"
             *   "Call +91 9876543210"
             *
             * Without this guard, an identifier followed by whitespace
             * and '+'/'-' is interpreted as a binary math expression.
             *
             * Compact expressions remain valid:
             *
             *   x+1
             *   a-2
             *
             * Explicit spaced binary expressions such as:
             *
             *   x + 1
             *
             * remain valid because whitespace follows the operator too.
             */
            if ((input[pos] == '+' ||
                 input[pos] == '-') &&
                pos > byte_offset &&
                IsSpace(input[pos - 1]) &&
                pos + 1 < input.size() &&
                !IsSpace(input[pos + 1]))
            {
                return {};
            }

            has_operator = true;
            ++pos;
            expect_operand = true;
            continue;
        }

        // Another operand without an operator.
        //
        // If whitespace separated a complete mathematical
        // expression from following prose, finish the Math
        // span before that whitespace:
        //
        //   "x+1 world" -> Math "x+1" + Word "world"
        //
        // But adjacent operands without an operator remain
        // invalid:
        //
        //   "1 2"
        //   "x y"
        //
        if (IsDigit(input[pos]) ||
            IsIdentifierStart(input[pos]) ||
            input[pos] == '(')
        {
            usize boundary = pos;

            while (boundary > byte_offset &&
                   IsSpace(input[boundary - 1]))
            {
                --boundary;
            }

            if (boundary < pos &&
                has_operator &&
                has_real_operand &&
                parentheses == 0)
            {
                pos = boundary;
                break;
            }

            return {};
        }

        break;
    }

    while (pos > byte_offset &&
           IsSpace(input[pos - 1]))
    {
        --pos;
    }

    if (!has_real_operand ||
        !has_operator ||
        expect_operand ||
        parentheses != 0 ||
        pos <= byte_offset)
    {
        return {};
    }

    if (!ValidBoundary(
            input,
            byte_offset,
            pos))
    {
        return {};
    }

    return RuleMatch{
        RuleType::Math,
        byte_offset,
        pos - byte_offset
    };
}

} // namespace qualix::rules
