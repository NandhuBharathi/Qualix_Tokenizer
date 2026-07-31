#include <string_view>

#include "core/platform.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::test;

int main()
{
    Platform platform = PlatformInfo::GetPlatform();

    Expect(
        platform == Platform::Windows ||
        platform == Platform::Linux ||
        platform == Platform::MacOS ||
        platform == Platform::Unknown,
        "PlatformInfo::GetPlatform()"
    );

    Compiler compiler = PlatformInfo::GetCompiler();

    Expect(
        compiler == Compiler::GCC ||
        compiler == Compiler::Clang ||
        compiler == Compiler::MSVC ||
        compiler == Compiler::Unknown,
        "PlatformInfo::GetCompiler()"
    );

    auto platform_name = PlatformInfo::PlatformName();

    Expect(
        platform_name == "Windows" ||
        platform_name == "Linux" ||
        platform_name == "macOS" ||
        platform_name == "Unknown",
        "PlatformInfo::PlatformName()"
    );

    auto compiler_name = PlatformInfo::CompilerName();

    Expect(
        compiler_name == "GCC" ||
        compiler_name == "Clang" ||
        compiler_name == "MSVC" ||
        compiler_name == "Unknown",
        "PlatformInfo::CompilerName()"
    );

#if defined(__linux__)
    Expect(platform == Platform::Linux,
           "Detected Linux");
    Expect(platform_name == "Linux",
           "Linux Name");
#endif

#if defined(__GNUC__) && !defined(__clang__)
    Expect(compiler == Compiler::GCC,
           "Detected GCC");
    Expect(compiler_name == "GCC",
           "GCC Name");
#endif

#if defined(__clang__)
    Expect(compiler == Compiler::Clang,
           "Detected Clang");
    Expect(compiler_name == "Clang",
           "Clang Name");
#endif

#if defined(_MSC_VER)
    Expect(compiler == Compiler::MSVC,
           "Detected MSVC");
    Expect(compiler_name == "MSVC",
           "MSVC Name");
#endif

    return Summary();
}
