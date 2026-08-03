
#include "rules/rule_engine.hpp"

#include <array>
#include <atomic>

#include <cctype>
#include <utility>

namespace qualix::rules
{

namespace router_profile
{

struct Counter
{
    std::atomic<unsigned long long> eligible{0};
    std::atomic<unsigned long long> skipped{0};
    std::atomic<unsigned long long> called{0};
    std::atomic<unsigned long long> matched{0};
};

constexpr usize kSlots = 32;

std::array<Counter, kSlots> counters{};

usize Index(RuleType type) noexcept
{
    return static_cast<usize>(type);
}

void Reset() noexcept
{
    for (auto& c : counters)
    {
        c.eligible.store(0, std::memory_order_relaxed);
        c.skipped.store(0, std::memory_order_relaxed);
        c.called.store(0, std::memory_order_relaxed);
        c.matched.store(0, std::memory_order_relaxed);
    }
}

void Eligible(RuleType type) noexcept
{
    const usize i = Index(type);

    if (i < counters.size())
        counters[i].eligible.fetch_add(
            1,
            std::memory_order_relaxed
        );
}

void Skipped(RuleType type) noexcept
{
    const usize i = Index(type);

    if (i < counters.size())
        counters[i].skipped.fetch_add(
            1,
            std::memory_order_relaxed
        );
}

void Called(RuleType type) noexcept
{
    const usize i = Index(type);

    if (i < counters.size())
        counters[i].called.fetch_add(
            1,
            std::memory_order_relaxed
        );
}

void Matched(RuleType type) noexcept
{
    const usize i = Index(type);

    if (i < counters.size())
        counters[i].matched.fetch_add(
            1,
            std::memory_order_relaxed
        );
}

unsigned long long GetEligible(
    RuleType type
) noexcept
{
    const usize i = Index(type);

    return i < counters.size()
        ? counters[i].eligible.load(
              std::memory_order_relaxed
          )
        : 0;
}

unsigned long long GetSkipped(
    RuleType type
) noexcept
{
    const usize i = Index(type);

    return i < counters.size()
        ? counters[i].skipped.load(
              std::memory_order_relaxed
          )
        : 0;
}

unsigned long long GetCalled(
    RuleType type
) noexcept
{
    const usize i = Index(type);

    return i < counters.size()
        ? counters[i].called.load(
              std::memory_order_relaxed
          )
        : 0;
}

unsigned long long GetMatched(
    RuleType type
) noexcept
{
    const usize i = Index(type);

    return i < counters.size()
        ? counters[i].matched.load(
              std::memory_order_relaxed
          )
        : 0;
}

} // namespace router_profile


namespace
{

bool IsAsciiDigit(
    unsigned char c
) noexcept
{
    return c >= '0' &&
           c <= '9';
}

bool IsAsciiLetter(
    unsigned char c
) noexcept
{
    return
        (c >= 'A' && c <= 'Z') ||
        (c >= 'a' && c <= 'z');
}

/*
 * Conservative routing.
 *
 * false means:
 *     this rule definitely cannot begin here.
 *
 * true does NOT mean it will match.
 * It only means the normal Rule::Match()
 * should be allowed to inspect the input.
 *
 * Ambiguous/non-ASCII input deliberately
 * falls back to existing rule behaviour.
 */
bool ShouldTry(
    RuleType type,
    unsigned char c,
    bool digit,
    bool letter
) noexcept
{
    /*
     * Non-ASCII may include Unicode currency
     * symbols and other meaningful prefixes.
     *
     * Preserve old semantics.
     */
    if (c >= 0x80)
        return true;

    switch (type)
    {
        case RuleType::Url:
            /*
             * Typical URL rules begin with
             * an ASCII scheme / www-like text.
             */
            return letter;

        case RuleType::Email:
            /*
             * Local-part commonly begins with
             * ASCII alphanumeric or permitted
             * punctuation.
             */
            return
                letter ||
                digit ||
                c == '_' ||
                c == '.' ||
                c == '+' ||
                c == '-';

        case RuleType::Date:
        case RuleType::Time:
        case RuleType::Number:
        case RuleType::Percentage:
        case RuleType::Measurement:
            /*
             * These numeric forms normally
             * begin with a digit/sign/decimal.
             *
             * Keep signs and decimal prefix
             * conservative.
             */
            return
                digit ||
                c == '+' ||
                c == '-' ||
                c == '.';

        case RuleType::Phone:
            return
                digit ||
                c == '+' ||
                c == '(';

        case RuleType::Currency:
            /*
             * Currency may be:
             *
             *   $100
             *   £100 / €100 / ₹100 (non-ASCII)
             *   100 USD
             *
             * Digits are retained because the
             * existing rule may support suffix
             * currency notation.
             */
            return
                letter ||
                digit ||
                c == '+' ||
                c == '-' ||
                c == '.' ||
                c == '$';

        case RuleType::Math:
            /*
             * Math may start with variables,
             * numbers, signs, brackets, etc.
             */
            return
                letter ||
                digit ||
                c == '+' ||
                c == '-' ||
                c == '.' ||
                c == '(' ||
                c == '[' ||
                c == '{';

        case RuleType::Code:
            /*
             * No CodeRule is currently installed,
             * but preserve generic behaviour.
             */
            return true;

        case RuleType::None:
            return false;
    }

    return true;
}


/*
 * #97 second-stage router.
 *
 * IMPORTANT:
 * false = rule definitely cannot begin here.
 * true  = rule is allowed to perform normal Match().
 *
 * This layer must remain conservative.
 */
bool ShouldTrySecondStage(
    RuleType type,
    std::string_view input,
    usize offset
) noexcept
{
    if (offset >= input.size())
        return false;

    const unsigned char c =
        static_cast<unsigned char>(
            input[offset]
        );

    /*
     * Unicode starts are deliberately left to
     * existing rule semantics.
     */
    if (c >= 0x80)
        return true;

    const bool digit =
        IsAsciiDigit(c);

    const bool letter =
        IsAsciiLetter(c);

    switch (type)
    {
        case RuleType::Url:
        {
            /*
             * Existing URL forms are ASCII textual
             * prefixes. A URL cannot start with an
             * arbitrary digit/punctuation here.
             *
             * First-stage routing already requires
             * letter, so preserve that contract.
             */
            return letter;
        }

        case RuleType::Email:
        {
            /*
             * Email local-part starts accepted by
             * the existing first-stage router.
             * Keep this conservative because an
             * address may begin with many letters.
             */
            return
                letter ||
                digit ||
                c == '_' ||
                c == '.' ||
                c == '+' ||
                c == '-';
        }

        case RuleType::Currency:
        {
            /*
             * Currency prefixes supported by the
             * current CurrencyRule include:
             *
             *   $, ISO codes, Rs/Rs.,
             *   Unicode currency symbols,
             *   and numeric suffix forms.
             *
             * Therefore ASCII candidates are:
             * digit/sign/dot/$/letter.
             */
            return
                digit ||
                letter ||
                c == '+' ||
                c == '-' ||
                c == '.' ||
                c == '$';
        }

        case RuleType::Percentage:
        case RuleType::Measurement:
        case RuleType::Date:
        case RuleType::Time:
        case RuleType::Number:
        {
            return
                digit ||
                c == '+' ||
                c == '-' ||
                c == '.';
        }

        case RuleType::Phone:
        {
            return
                digit ||
                c == '+' ||
                c == '(';
        }

        case RuleType::Math:
        {
            /*
             * Math is intentionally broad.
             * Do not over-filter variables.
             */
            return
                letter ||
                digit ||
                c == '+' ||
                c == '-' ||
                c == '.' ||
                c == '(' ||
                c == '[' ||
                c == '{';
        }

        case RuleType::Code:
            return true;

        case RuleType::None:
            return false;
    }

    return true;
}


/*
 * #98 prefix-hint router.
 *
 * Conservative look-ahead layer.
 *
 * false = definitely impossible
 * true  = allow existing Rule::Match().
 *
 * No semantic decision is made here.
 */
bool ShouldTryPrefixHint(
    RuleType type,
    std::string_view input,
    usize offset
) noexcept
{
    if (offset >= input.size())
        return false;

    const unsigned char c0 =
        static_cast<unsigned char>(
            input[offset]
        );

    /*
     * Preserve Unicode behaviour.
     */
    if (c0 >= 0x80)
        return true;

    const bool digit0 =
        IsAsciiDigit(c0);

    const bool letter0 =
        IsAsciiLetter(c0);

    const usize remaining =
        input.size() - offset;

    switch (type)
    {
        case RuleType::Url:
        {
            /*
             * URL must begin textually.
             *
             * Do not assume only http/www:
             * existing UrlRule may support
             * additional schemes.
             */
            if (!letter0)
                return false;

            /*
             * Cheap scan for URL structural
             * evidence in a bounded prefix.
             */
            const usize limit =
                remaining < 32
                ? remaining
                : 32;

            for (usize i = 1; i < limit; ++i)
            {
                const unsigned char c =
                    static_cast<unsigned char>(
                        input[offset + i]
                    );

                if (c == ':' ||
                    c == '.' ||
                    c == '/')
                {
                    return true;
                }

                if (c == ' ' ||
                    c == '\t' ||
                    c == '\n' ||
                    c == '\r')
                {
                    break;
                }

                if (c >= 0x80)
                    return true;
            }

            /*
             * No URL evidence in bounded
             * lookahead.
             */
            return false;
        }

        case RuleType::Email:
        {
            /*
             * Search a bounded local token for @.
             *
             * Existing first-stage router already
             * validates possible local-part start.
             */
            const usize limit =
                remaining < 64
                ? remaining
                : 64;

            for (usize i = 1; i < limit; ++i)
            {
                const unsigned char c =
                    static_cast<unsigned char>(
                        input[offset + i]
                    );

                if (c == '@')
                    return true;

                if (c == ' ' ||
                    c == '\t' ||
                    c == '\n' ||
                    c == '\r')
                {
                    return false;
                }

                /*
                 * Unicode ambiguity:
                 * preserve normal rule.
                 */
                if (c >= 0x80)
                    return true;
            }

            return false;
        }

        case RuleType::Currency:
        {
            /*
             * Numeric starts may represent
             * suffix currency:
             *
             * 500 USD
             */
            if (digit0 ||
                c0 == '+' ||
                c0 == '-' ||
                c0 == '.' ||
                c0 == '$')
            {
                return true;
            }

            /*
             * ASCII textual prefixes:
             * Rs, USD, EUR, INR, etc.
             *
             * CurrencyRule itself remains the
             * authority.
             */
            if (letter0)
            {
                if (remaining >= 2)
                {
                    const unsigned char c1 =
                        static_cast<unsigned char>(
                            input[offset + 1]
                        );

                    /*
                     * Rs / ISO-like text.
                     */
                    if (IsAsciiLetter(c1))
                        return true;
                }

                return false;
            }

            return false;
        }

        case RuleType::Percentage:
        {
            /*
             * Percentage must start numerically.
             */
            return
                digit0 ||
                c0 == '+' ||
                c0 == '-' ||
                c0 == '.';
        }

        case RuleType::Measurement:
        {
            return
                digit0 ||
                c0 == '+' ||
                c0 == '-' ||
                c0 == '.';
        }

        case RuleType::Date:
        case RuleType::Time:
        case RuleType::Number:
        {
            return
                digit0 ||
                c0 == '+' ||
                c0 == '-' ||
                c0 == '.';
        }

        case RuleType::Phone:
        {
            return
                digit0 ||
                c0 == '+' ||
                c0 == '(';
        }

        case RuleType::Math:
        {
            /*
             * Math remains deliberately broad.
             * Variables make aggressive filtering
             * unsafe without inspecting MathRule.
             */
            return true;
        }

        case RuleType::Code:
            return true;

        case RuleType::None:
            return false;
    }

    return true;
}

} // namespace

void RuleEngine::Add(
    std::unique_ptr<Rule> rule
)
{
    if (rule)
    {
        rules_.push_back(
            std::move(rule)
        );
    }
}

RuleMatch RuleEngine::MatchAt(
    std::string_view input,
    usize byte_offset
) const noexcept
{
    if (byte_offset >= input.size())
        return {};

    RuleMatch best{};

    /*
     * #93 router fast-path.
     *
     * First-byte classification belongs to the
     * routing position, not to each individual
     * rule. Compute it once and reuse it.
     */
    const unsigned char c =
        static_cast<unsigned char>(
            input[byte_offset]
        );

    const bool digit =
        c < 0x80 &&
        IsAsciiDigit(c);

    const bool letter =
        c < 0x80 &&
        IsAsciiLetter(c);

    for (const auto& rule : rules_)
    {
        const RuleType type =
            rule->Type();

        if (!ShouldTry(
                type,
                c,
                digit,
                letter))
        {
            (void)0;
            continue;
        }

        /* #97_SECOND_STAGE_CALL */
        if (!ShouldTrySecondStage(
                type,
                input,
                byte_offset))
        {
            (void)0;
            continue;
        }

        /* #98_PREFIX_HINT_CALL */
        if (!ShouldTryPrefixHint(
                type,
                input,
                byte_offset))
        {
            (void)0;
            continue;
        }

        (void)0;
        (void)0;

        const RuleMatch match =
            rule->Match(
                input,
                byte_offset
            );

        if (!match.Matched())
            continue;

        (void)0;

        if (match.byte_start !=
            byte_offset)
        {
            continue;
        }

        if (match.ByteEnd() >
            input.size())
        {
            continue;
        }

        if (!best.Matched() ||
            match.byte_length >
                best.byte_length)
        {
            best = match;
        }
    }

    return best;
}

} // namespace qualix::rules
