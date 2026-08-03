#include "bpe/byte_fallback.hpp"

namespace qualix::bpe
{

std::string ByteFallback::Symbol(
    u8 byte
)
{
    static constexpr char Hex[] =
        "0123456789ABCDEF";

    std::string symbol;

    symbol.reserve(6);

    symbol.push_back('<');
    symbol.push_back('0');
    symbol.push_back('x');

    symbol.push_back(
        Hex[(byte >> 4) & 0x0Fu]
    );

    symbol.push_back(
        Hex[byte & 0x0Fu]
    );

    symbol.push_back('>');

    return symbol;
}

int ByteFallback::HexValue(
    char value
) noexcept
{
    if (value >= '0' &&
        value <= '9')
    {
        return value - '0';
    }

    if (value >= 'A' &&
        value <= 'F')
    {
        return
            value - 'A' + 10;
    }

    if (value >= 'a' &&
        value <= 'f')
    {
        return
            value - 'a' + 10;
    }

    return -1;
}

bool ByteFallback::ParseSymbol(
    std::string_view symbol,
    u8& byte
) noexcept
{
    if (symbol.size() != 6)
        return false;

    if (symbol[0] != '<' ||
        symbol[1] != '0' ||
        symbol[2] != 'x' ||
        symbol[5] != '>')
    {
        return false;
    }

    const int high =
        HexValue(symbol[3]);

    const int low =
        HexValue(symbol[4]);

    if (high < 0 ||
        low < 0)
    {
        return false;
    }

    byte =
        static_cast<u8>(
            (high << 4) |
            low
        );

    return true;
}

std::vector<std::string>
ByteFallback::Encode(
    std::string_view text
)
{
    std::vector<std::string> result;

    result.reserve(
        text.size()
    );

    for (const unsigned char byte : text)
    {
        result.push_back(
            Symbol(
                static_cast<u8>(
                    byte
                )
            )
        );
    }

    return result;
}

std::string ByteFallback::Decode(
    std::span<const std::string> symbols
)
{
    std::string result;

    result.reserve(
        symbols.size()
    );

    for (const auto& symbol : symbols)
    {
        u8 byte = 0;

        if (!ParseSymbol(
                symbol,
                byte))
        {
            return {};
        }

        result.push_back(
            static_cast<char>(
                byte
            )
        );
    }

    return result;
}

} // namespace qualix::bpe
