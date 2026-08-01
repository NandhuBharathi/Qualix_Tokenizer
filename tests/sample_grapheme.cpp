#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "unicode/grapheme_segmenter.hpp"

using namespace qualix;
using namespace qualix::unicode;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: sample_grapheme <file>\n";
        return 1;
    }

    std::ifstream file(argv[1], std::ios::binary);

    if (!file)
    {
        std::cerr << "Failed to open: " << argv[1] << "\n";
        return 1;
    }

    std::string text{
        std::istreambuf_iterator<char>{file},
        std::istreambuf_iterator<char>{}
    };

    auto result = GraphemeSegmenter::Segment(text);

    if (result.Failed())
    {
        std::cerr << "Segmentation failed\n";
        return 1;
    }

    std::cout << "=== INPUT ===\n\n";
    std::cout << text << "\n";

    std::cout << "=== GRAPHEMES ===\n\n";

    usize index = 0;

    for (const auto& grapheme : result.Value())
    {
        const auto view = grapheme.View(text);

        std::cout << index++ << " : [";

        if (view == "\n")
            std::cout << "\\n";
        else if (view == "\r")
            std::cout << "\\r";
        else if (view == "\t")
            std::cout << "\\t";
        else if (view == " ")
            std::cout << "<SPACE>";
        else
            std::cout << view;

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
