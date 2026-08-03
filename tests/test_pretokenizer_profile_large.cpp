
#include <iostream>
#include <string>
#include "pretokenizer/pretokenizer.hpp"

using namespace qualix;

int main()
{
    const std::string unit =
        "Hello world 12345 test@example.com "
        "https://example.com/path 12.5% $99.50 "
        "2026-08-02 10:30 +91-9876543210 "
        "25kg x+y=10 தமிழ் வணக்கம் ";

    std::string corpus;
    corpus.reserve(unit.size() * 1500);

    for (int i = 0; i < 1500; ++i)
        corpus += unit;

    std::cout
        << "============================================================\n"
        << "QUALIX — #87.1 LARGE PRETOKENIZER PROFILE\n"
        << "============================================================\n"
        << "Corpus : "
        << corpus.size() / 1000000.0
        << " MB\n";

    auto result =
        pretokenizer::PreTokenizer::Split(
            corpus
        );

    if (result.Failed())
    {
        std::cout << "[FAIL] PRETOKENIZER\n";
        return 1;
    }

    std::cout
        << "Output spans : "
        << result.Value().size()
        << '\n'
        << "============================================================\n";

    return 0;
}
