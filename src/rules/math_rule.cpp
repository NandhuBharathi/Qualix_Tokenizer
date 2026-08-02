#include "rules/math_rule.hpp"

namespace qualix::rules
{

namespace
{

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

bool ParseNumber(
    std::string_view input,
    usize& pos
) noexcept
{
    const usize begin = pos;

    bool has_digit = false;

    while (pos < input.size() &&
           IsDigit(input[pos]))
    {
        has_digit = true;
        ++pos;
    }

    if (pos < input.size() &&
        input[pos] == '.')
    {
        ++pos;

        const usize fraction_begin = pos;

        while (pos < input.size() &&
               IsDigit(input[pos]))
        {
            has_digit = true;
            ++pos;
        }

        if (pos == fraction_begin)
        {
            pos = begin;
            return false;
        }
    }

    if (!has_digit)
    {
        pos = begin;
        return false;
    }

    return true;
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
        return ParseNumber(input, pos);

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
            has_operator = true;
            ++pos;
            expect_operand = true;
            continue;
        }

        // Another operand without an operator:
        // "1 2", "x y", etc.
        if (IsDigit(input[pos]) ||
            IsIdentifierStart(input[pos]) ||
            input[pos] == '(')
        {
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
