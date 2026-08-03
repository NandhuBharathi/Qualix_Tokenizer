#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>

#include "core/version.hpp"
#include "pretokenizer/pretokenizer.hpp"
#include "unicode/grapheme_segmenter.hpp"
#include "unicode/normalizer.hpp"

using namespace qualix;
using namespace qualix::unicode;
using namespace qualix::pretokenizer;

namespace
{

Result<std::string> ReadFile(
    const std::string& path
)
{
    std::ifstream file(
        path,
        std::ios::binary
    );

    if (!file)
        return Status::Failure(
            ErrorCode::InvalidArgument
        );

    return std::string{
        std::istreambuf_iterator<char>{file},
        std::istreambuf_iterator<char>{}
    };
}

void WriteText(
    std::ostream& output,
    std::string_view text
)
{
    if (text == "\n")
        output << "\\n";
    else if (text == "\r")
        output << "\\r";
    else if (text == "\t")
        output << "\\t";
    else if (text == " ")
        output << "<SPACE>";
    else
        output << text;
}

void PrintText(
    std::string_view text
)
{
    WriteText(
        std::cout,
        text
    );
}

std::filesystem::path MakeOutputPath(
    const std::string& input_path
)
{
    const std::filesystem::path input{
        input_path
    };

    const auto stem =
        input.stem().string();

    // Kaggle working root.
    // Example:
    // sample.txt -> /kaggle/working/sample.pretokens.txt
    const std::filesystem::path kaggle_root{
        "/kaggle/working"
    };

    if (std::filesystem::exists(kaggle_root))
    {
        return kaggle_root /
            (stem + ".pretokens.txt");
    }

    // Portable fallback outside Kaggle.
    const auto parent =
        input.parent_path();

    return parent /
        (stem + ".pretokens.txt");
}

void PrintUsage()
{
    std::cout
        << "Qualix Tokenizer "
        << Version::String()
        << "\n\n"
        << "Usage:\n"
        << "  qualix --version\n"
        << "  qualix inspect <file>\n"
        << "  qualix pretokenize <file>\n";
}

int InspectFile(
    const std::string& path
)
{
    auto loaded = ReadFile(path);

    if (loaded.Failed())
    {
        std::cerr
            << "Error: failed to open file: "
            << path
            << "\n";

        return 1;
    }

    const std::string& text =
        loaded.Value();

    auto result =
        GraphemeSegmenter::Segment(text);

    if (result.Failed())
    {
        std::cerr
            << "Error: grapheme segmentation failed\n";

        return 1;
    }

    std::cout
        << "=== Qualix Grapheme Inspection ===\n\n";

    usize index = 0;

    for (const auto& grapheme :
         result.Value())
    {
        const auto view =
            grapheme.View(text);

        std::cout
            << index++
            << " : [";

        PrintText(view);

        std::cout
            << "] bytes="
            << grapheme.byte_length
            << "\n";
    }

    std::cout
        << "\nTotal bytes     : "
        << text.size()
        << "\n"
        << "Total graphemes : "
        << result.Value().size()
        << "\n";

    return 0;
}

int PretokenizeFile(
    const std::string& path
)
{
    auto loaded = ReadFile(path);

    if (loaded.Failed())
    {
        std::cerr
            << "Error: failed to open file: "
            << path
            << "\n";

        return 1;
    }

    const std::string& input =
        loaded.Value();

    auto normalized =
        Normalizer::Normalize(
            input,
            NormalizationForm::NFC
        );

    if (normalized.Failed())
    {
        std::cerr
            << "Error: NFC normalization failed\n";

        return 1;
    }

    const std::string& text =
        normalized.Value();

    auto result =
        PreTokenizer::Split(text);

    if (result.Failed())
    {
        std::cerr
            << "Error: pre-tokenization failed\n";

        return 1;
    }

    const auto& spans =
        result.Value();

    const auto output_path =
        MakeOutputPath(path);

    std::ofstream output(
        output_path,
        std::ios::binary |
        std::ios::trunc
    );

    if (!output)
    {
        std::cerr
            << "Error: failed to create output file: "
            << output_path.string()
            << "\n";

        return 1;
    }

    output
        << "================================\n"
        << "Qualix PreTokenizer\n"
        << "================================\n\n"
        << "Input file       : "
        << path
        << "\n"
        << "Input bytes      : "
        << input.size()
        << "\n"
        << "Normalized bytes : "
        << text.size()
        << "\n"
        << "Normalization    : NFC\n\n"
        << "=== PRETOKENS ===\n\n";

    for (usize i = 0;
         i < spans.size();
         ++i)
    {
        const auto& span =
            spans[i];

        const auto view =
            span.View(text);

        output
            << i
            << " : [";

        WriteText(
            output,
            view
        );

        output
            << "]"
            << " type="
            << ToString(span.type)
            << " bytes="
            << span.byte_length
            << " graphemes="
            << span.grapheme_count
            << " byte_range=["
            << span.byte_start
            << ","
            << span.ByteEnd()
            << ")"
            << " grapheme_range=["
            << span.grapheme_start
            << ","
            << span.GraphemeEnd()
            << ")"
            << "\n";
    }

    output
        << "\n================================\n"
        << "Summary\n"
        << "================================\n"
        << "Input bytes      : "
        << input.size()
        << "\n"
        << "Normalized bytes : "
        << text.size()
        << "\n"
        << "PreToken spans   : "
        << spans.size()
        << "\n"
        << "================================\n";

    output.close();

    if (!output)
    {
        std::cerr
            << "Error: failed while writing output file\n";

        return 1;
    }

    std::cout
        << "================================\n"
        << "Qualix PreTokenizer\n"
        << "================================\n"
        << "Input  : "
        << path
        << "\n"
        << "Output : "
        << output_path.string()
        << "\n"
        << "Spans  : "
        << spans.size()
        << "\n"
        << "Status : SUCCESS\n"
        << "================================\n";

    return 0;
}

} // namespace

int main(
    int argc,
    char** argv
)
{
    if (argc == 2)
    {
        const std::string_view command =
            argv[1];

        if (command == "--version" ||
            command == "-v")
        {
            std::cout
                << Version::Name
                << " "
                << Version::String()
                << "\n";

            return 0;
        }

        if (command == "--help" ||
            command == "-h")
        {
            PrintUsage();
            return 0;
        }
    }

    if (argc == 3)
    {
        const std::string_view command =
            argv[1];

        if (command == "inspect")
            return InspectFile(argv[2]);

        if (command == "pretokenize")
            return PretokenizeFile(argv[2]);
    }

    PrintUsage();
    return 1;
}
