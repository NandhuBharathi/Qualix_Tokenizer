#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "unicode/normalizer.hpp"
#include "unicode/utf8.hpp"

using namespace qualix;
using namespace qualix::unicode;

namespace
{

std::string Trim(std::string value)
{
    const auto first =
        value.find_first_not_of(" \t\r\n");

    if (first == std::string::npos)
        return {};

    const auto last =
        value.find_last_not_of(" \t\r\n");

    return value.substr(
        first,
        last - first + 1
    );
}

bool ParseSequence(
    const std::string& field,
    std::string& output
)
{
    output.clear();

    std::istringstream stream(field);
    std::string token;

    while (stream >> token)
    {
        CodePoint cp = 0;

        try
        {
            cp = static_cast<CodePoint>(
                std::stoul(
                    token,
                    nullptr,
                    16
                )
            );
        }
        catch (...)
        {
            return false;
        }

        auto encoded = Utf8::Encode(cp);

        if (encoded.Failed())
            return false;

        output += encoded.Value();
    }

    return true;
}

bool ParseLine(
    const std::string& raw,
    std::vector<std::string>& columns
)
{
    std::string line = raw;

    const auto comment =
        line.find('#');

    if (comment != std::string::npos)
        line.erase(comment);

    line = Trim(line);

    if (line.empty() ||
        line.front() == '@')
    {
        return false;
    }

    columns.clear();

    std::stringstream stream(line);
    std::string field;

    while (std::getline(
        stream,
        field,
        ';'
    ))
    {
        columns.push_back(
            Trim(field)
        );
    }

    if (columns.size() < 5)
        return false;

    columns.resize(5);

    return true;
}

bool CheckNfkd(
    std::string_view input,
    std::string_view expected
)
{
    auto result =
        Normalizer::Normalize(
            input,
            NormalizationForm::NFKD
        );

    return result.Ok() &&
           result.Value() == expected;
}

void PrintHex(
    std::string_view text
)
{
    usize offset = 0;

    while (offset < text.size())
    {
        auto decoded =
            Utf8::Decode(
                text.substr(offset)
            );

        if (decoded.Failed())
        {
            std::cerr << "<invalid>";
            return;
        }

        const auto value =
            decoded.Value();

        std::cerr
            << "U+"
            << std::hex
            << std::uppercase
            << value.codepoint
            << std::dec
            << " ";

        offset +=
            value.bytes_consumed;
    }
}

} // namespace

int main()
{
    const char* path =
        "/kaggle/working/Qualix_Tokenizer/"
        "tools/unicode/data/"
        "NormalizationTest.txt";

    std::ifstream file(path);

    if (!file)
    {
        std::cerr
            << "[FAIL] Unable to open "
            << "NormalizationTest.txt\n";

        return 1;
    }

    usize cases = 0;
    usize checks = 0;
    usize passed = 0;
    usize failed = 0;
    usize line_number = 0;

    std::string line;
    std::vector<std::string> fields;

    while (std::getline(file, line))
    {
        ++line_number;

        if (!ParseLine(
                line,
                fields))
        {
            continue;
        }

        std::string c1;
        std::string c2;
        std::string c3;
        std::string c4;
        std::string c5;

        if (!ParseSequence(fields[0], c1) ||
            !ParseSequence(fields[1], c2) ||
            !ParseSequence(fields[2], c3) ||
            !ParseSequence(fields[3], c4) ||
            !ParseSequence(fields[4], c5))
        {
            std::cerr
                << "[FAIL] Parse error line "
                << line_number
                << "\n";

            return 1;
        }

        ++cases;

        struct Check
        {
            const char* name;
            const std::string* input;
        };

        const Check test_checks[] =
        {
            {"NFKD(c1)=c5", &c1},
            {"NFKD(c2)=c5", &c2},
            {"NFKD(c3)=c5", &c3},
            {"NFKD(c4)=c5", &c4},
            {"NFKD(c5)=c5", &c5}
        };

        for (const auto& check :
             test_checks)
        {
            ++checks;

            if (CheckNfkd(
                    *check.input,
                    c5))
            {
                ++passed;
                continue;
            }

            ++failed;

            std::cerr
                << "[FAIL] Line "
                << line_number
                << " "
                << check.name
                << "\n";

            auto actual =
                Normalizer::Normalize(
                    *check.input,
                    NormalizationForm::NFKD
                );

            std::cerr
                << "       Input   : ";

            PrintHex(*check.input);

            std::cerr
                << "\n       Expected: ";

            PrintHex(c5);

            std::cerr
                << "\n       Actual  : ";

            if (actual.Ok())
                PrintHex(actual.Value());
            else
                std::cerr
                    << "<normalization failed>";

            std::cerr << "\n";

            if (failed >= 20)
            {
                std::cerr
                    << "\nStopped after "
                    << "first 20 failures.\n";

                goto finished;
            }
        }
    }

finished:

    std::cout
        << "\n"
        << "================================\n"
        << "Unicode 17 NFKD Conformance\n"
        << "================================\n"
        << "Test Cases    : "
        << cases << "\n"
        << "Checks Run    : "
        << checks << "\n"
        << "Checks Passed : "
        << passed << "\n"
        << "Checks Failed : "
        << failed << "\n"
        << "================================\n";

    return failed == 0 ? 0 : 1;
}
