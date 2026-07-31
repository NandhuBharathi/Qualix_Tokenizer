#include <string_view>

#include "core/version.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::test;

int main()
{
    Expect(Version::Major == 0,
           "Version::Major");

    Expect(Version::Minor == 1,
           "Version::Minor");

    Expect(Version::Patch == 0,
           "Version::Patch");

    Expect(Version::Name == "Qualix Tokenizer",
           "Version::Name");

    Expect(Version::String() == "0.1.0",
           "Version::String");

    return Summary();
}
