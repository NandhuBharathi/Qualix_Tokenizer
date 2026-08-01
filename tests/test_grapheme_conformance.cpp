#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "unicode/grapheme_segmenter.hpp"
#include "unicode/utf8.hpp"

using namespace qualix;
using namespace qualix::unicode;

namespace
{

struct TestCase
{
    std::string text;
    std::vector<usize> boundaries;
};

bool ParseTestCase(
    const std::string& raw_line,
    TestCase& test
)
{
    std::string line = raw_line;

    const auto comment = line.find('#');

    if (comment != std::string::npos)
        line.erase(comment);

    std::istringstream stream(line);
    std::string token;

    std::vector<CodePoint> codepoints;
    std::vector<bool> breaks;

    bool next_is_break = false;
    bool have_marker = false;

    while (stream >> token)
    {
        if (token == "÷")
        {
            next_is_break = true;
            have_marker = true;
            continue;
        }

        if (token == "×")
        {
            next_is_break = false;
            have_marker = true;
            continue;
        }

        if (!have_marker)
            return false;

        CodePoint cp = 0;

        try
        {
            cp = static_cast<CodePoint>(
                std::stoul(token, nullptr, 16)
            );
        }
        catch (...)
        {
            return false;
        }

        codepoints.push_back(cp);
        breaks.push_back(next_is_break);

        have_marker = false;
    }

    if (codepoints.empty())
        return false;

    test.text.clear();
    test.boundaries.clear();

    usize byte_offset = 0;

    for (usize i = 0; i < codepoints.size(); ++i)
    {
        if (breaks[i])
            test.boundaries.push_back(byte_offset);

        auto encoded = Utf8::Encode(codepoints[i]);

        if (encoded.Failed())
            return false;

        test.text += encoded.Value();
        byte_offset += encoded.Value().size();
    }

    test.boundaries.push_back(byte_offset);

    return true;
}

std::vector<usize> ActualBoundaries(
    const std::string& text
)
{
    std::vector<usize> boundaries;

    auto result = GraphemeSegmenter::Segment(text);

    if (result.Failed())
        return boundaries;

    boundaries.push_back(0);

    for (const auto& grapheme : result.Value())
        boundaries.push_back(grapheme.ByteEnd());

    return boundaries;
}

void PrintBoundaries(
    const std::vector<usize>& boundaries
)
{
    std::cerr << "[";

    for (usize i = 0; i < boundaries.size(); ++i)
    {
        if (i != 0)
            std::cerr << ", ";

        std::cerr << boundaries[i];
    }

    std::cerr << "]";
}

} // namespace

int main()
{
    const char* path =
        "/kaggle/working/Qualix_Tokenizer/"
        "tools/unicode/data/GraphemeBreakTest.txt";

    std::ifstream file(path);

    if (!file)
    {
        std::cerr
            << "[FAIL] Unable to open GraphemeBreakTest.txt\n";

        return 1;
    }

    usize parsed = 0;
    usize passed = 0;
    usize failed = 0;
    usize line_number = 0;

    std::string line;

    while (std::getline(file, line))
    {
        ++line_number;

        TestCase test;

        if (!ParseTestCase(line, test))
            continue;

        ++parsed;

        const auto actual =
            ActualBoundaries(test.text);

        if (actual == test.boundaries)
        {
            ++passed;
            continue;
        }

        ++failed;

        std::cerr
            << "[FAIL] Unicode test line "
            << line_number
            << "\n";

        std::cerr << "       Expected: ";
        PrintBoundaries(test.boundaries);

        std::cerr << "\n       Actual  : ";
        PrintBoundaries(actual);

        std::cerr << "\n";

        if (failed >= 20)
        {
            std::cerr
                << "\nStopped after first 20 failures.\n";
            break;
        }
    }

    std::cout << "\n"
              << "================================\n"
              << "Unicode Grapheme Conformance\n"
              << "================================\n"
              << "Tests Parsed : " << parsed << "\n"
              << "Tests Passed : " << passed << "\n"
              << "Tests Failed : " << failed << "\n"
              << "================================\n";

    return failed == 0 ? 0 : 1;
}
