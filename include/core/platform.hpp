#pragma once

#include <string_view>

namespace qualix
{

enum class Platform
{
    Unknown,
    Windows,
    Linux,
    MacOS
};

enum class Compiler
{
    Unknown,
    GCC,
    Clang,
    MSVC
};

class PlatformInfo
{
public:
    static constexpr Platform GetPlatform() noexcept
    {
#if defined(_WIN32)
        return Platform::Windows;
#elif defined(__linux__)
        return Platform::Linux;
#elif defined(__APPLE__)
        return Platform::MacOS;
#else
        return Platform::Unknown;
#endif
    }

    static constexpr Compiler GetCompiler() noexcept
    {
#if defined(__clang__)
        return Compiler::Clang;
#elif defined(__GNUC__)
        return Compiler::GCC;
#elif defined(_MSC_VER)
        return Compiler::MSVC;
#else
        return Compiler::Unknown;
#endif
    }

    static constexpr std::string_view PlatformName() noexcept
    {
        switch (GetPlatform())
        {
            case Platform::Windows: return "Windows";
            case Platform::Linux:   return "Linux";
            case Platform::MacOS:   return "macOS";
            default:                return "Unknown";
        }
    }

    static constexpr std::string_view CompilerName() noexcept
    {
        switch (GetCompiler())
        {
            case Compiler::GCC:   return "GCC";
            case Compiler::Clang: return "Clang";
            case Compiler::MSVC:  return "MSVC";
            default:              return "Unknown";
        }
    }
};

} // namespace qualix
