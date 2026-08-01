#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "unicode/general_category.hpp"

using namespace qualix;
using namespace qualix::unicode;

namespace
{

bool ParseCategory(
    const std::string& value,
    GeneralCategory& category
)
{
    #define QUALIX_CATEGORY(name) \
        if (value == #name)       \
        {                         \
            category = GeneralCategory::name; \
            return true;          \
        }

    QUALIX_CATEGORY(Lu)
    QUALIX_CATEGORY(Ll)
    QUALIX_CATEGORY(Lt)
    QUALIX_CATEGORY(Lm)
    QUALIX_CATEGORY(Lo)

    QUALIX_CATEGORY(Mn)
    QUALIX_CATEGORY(Mc)
    QUALIX_CATEGORY(Me)

    QUALIX_CATEGORY(Nd)
    QUALIX_CATEGORY(Nl)
    QUALIX_CATEGORY(No)

    QUALIX_CATEGORY(Pc)
    QUALIX_CATEGORY(Pd)
    QUALIX_CATEGORY(Ps)
    QUALIX_CATEGORY(Pe)
    QUALIX_CATEGORY(Pi)
    QUALIX_CATEGORY(Pf)
    QUALIX_CATEGORY(Po)

    QUALIX_CATEGORY(Sm)
    QUALIX_CATEGORY(Sc)
    QUALIX_CATEGORY(Sk)
    QUALIX_CATEGORY(So)

    QUALIX_CATEGORY(Zs)
    QUALIX_CATEGORY(Zl)
    QUALIX_CATEGORY(Zp)

    QUALIX_CATEGORY(Cc)
    QUALIX_CATEGORY(Cf)
    QUALIX_CATEGORY(Cs)
    QUALIX_CATEGORY(Co)

    #undef QUALIX_CATEGORY

    return false;
}

bool ParseRecord(
    const std::string& line,
    CodePoint& codepoint,
    std::string& name,
    GeneralCategory& category
)
{
    std::stringstream stream(line);
    std::string cp_field;
    std::string category_field;

    if (!std::getline(stream, cp_field, ';'))
        return false;

    if (!std::getline(stream, name, ';'))
        return false;

    if (!std::getline(stream, category_field, ';'))
        return false;

    try
    {
        codepoint = static_cast<CodePoint>(
            std::stoul(
                cp_field,
                nullptr,
                16
            )
        );
    }
    catch (...)
    {
        return false;
    }

    return ParseCategory(
        category_field,
        category
    );
}

bool EndsWith(
    const std::string& text,
    const std::string& suffix
)
{
    if (text.size() < suffix.size())
        return false;

    return text.compare(
        text.size() - suffix.size(),
        suffix.size(),
        suffix
    ) == 0;
}

} // namespace

int main()
{
    const char* path =
        "/kaggle/working/Qualix_Tokenizer/"
        "tools/unicode/data/UnicodeData.txt";

    std::ifstream file(path);

    if (!file)
    {
        std::cerr
            << "[FAIL] Unable to open UnicodeData.txt\n";

        return 1;
    }

    usize source_records = 0;
    usize checks = 0;
    usize passed = 0;
    usize failed = 0;

    bool range_open = false;
    CodePoint range_first = 0;
    GeneralCategory range_category =
        GeneralCategory::Cn;

    std::string line;
    usize line_number = 0;

    while (std::getline(file, line))
    {
        ++line_number;

        if (line.empty())
            continue;

        CodePoint cp = 0;
        std::string name;
        GeneralCategory category =
            GeneralCategory::Cn;

        if (!ParseRecord(
                line,
                cp,
                name,
                category))
        {
            std::cerr
                << "[FAIL] Parse error at line "
                << line_number
                << "\n";

            return 1;
        }

        ++source_records;

        if (EndsWith(name, ", First>"))
        {
            if (range_open)
            {
                std::cerr
                    << "[FAIL] Nested range at line "
                    << line_number
                    << "\n";

                return 1;
            }

            range_open = true;
            range_first = cp;
            range_category = category;

            continue;
        }

        if (EndsWith(name, ", Last>"))
        {
            if (!range_open)
            {
                std::cerr
                    << "[FAIL] Last without First at line "
                    << line_number
                    << "\n";

                return 1;
            }

            if (category != range_category ||
                cp < range_first)
            {
                std::cerr
                    << "[FAIL] Invalid range at line "
                    << line_number
                    << "\n";

                return 1;
            }

            for (CodePoint current = range_first;
                 current <= cp;
                 ++current)
            {
                ++checks;

                if (GeneralCategoryOf(current) ==
                    range_category)
                {
                    ++passed;
                }
                else
                {
                    ++failed;

                    if (failed <= 20)
                    {
                        std::cerr
                            << "[FAIL] U+"
                            << std::hex
                            << std::uppercase
                            << current
                            << std::dec
                            << " expected "
                            << ToString(range_category)
                            << " got "
                            << ToString(
                                GeneralCategoryOf(current)
                            )
                            << "\n";
                    }
                }
            }

            range_open = false;
            continue;
        }

        if (range_open)
        {
            std::cerr
                << "[FAIL] Range not closed before line "
                << line_number
                << "\n";

            return 1;
        }

        ++checks;

        const auto actual =
            GeneralCategoryOf(cp);

        if (actual == category)
        {
            ++passed;
        }
        else
        {
            ++failed;

            if (failed <= 20)
            {
                std::cerr
                    << "[FAIL] U+"
                    << std::hex
                    << std::uppercase
                    << cp
                    << std::dec
                    << " expected "
                    << ToString(category)
                    << " got "
                    << ToString(actual)
                    << "\n";
            }
        }
    }

    if (range_open)
    {
        std::cerr
            << "[FAIL] UnicodeData ended with open range\n";

        return 1;
    }

    std::cout
        << "\n"
        << "================================\n"
        << "Unicode 17 General Category\n"
        << "================================\n"
        << "Source Records : "
        << source_records << "\n"
        << "Checks Run     : "
        << checks << "\n"
        << "Checks Passed  : "
        << passed << "\n"
        << "Checks Failed  : "
        << failed << "\n"
        << "================================\n";

    return failed == 0 ? 0 : 1;
}
