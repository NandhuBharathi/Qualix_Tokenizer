#include <iostream>
#include <string>

#include "pretokenizer/pretokenizer.hpp"
#include "pretokenizer/span_type.hpp"

int main()
{
    const std::string text =
        "123 50% 12/3/25 "
        "₹500 user@example.com "
        "https://example.com";

    const auto result =
        qualix::pretokenizer::PreTokenizer::Split(text);

    if (!result.Ok())
    {
        std::cout << "Split failed\n";
        return 1;
    }

    for (const auto& span : result.Value())
    {
        std::cout
            << "["
            << span.View(text)
            << "] type="
            << qualix::pretokenizer::ToString(span.type)
            << " protected="
            << span.Protected()
            << '\n';
    }

    return 0;
}
