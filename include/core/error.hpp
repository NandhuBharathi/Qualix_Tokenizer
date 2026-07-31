#pragma once

#include <string_view>

namespace qualix
{

enum class ErrorCode
{
    // Success
    None = 0,

    // General
    Unknown,
    InvalidArgument,
    InvalidState,
    NotImplemented,

    // File System
    FileNotFound,
    FileOpenFailed,
    FileReadFailed,
    FileWriteFailed,
    DirectoryNotFound,

    // Unicode
    InvalidUtf8,
    InvalidUtf16,
    InvalidUtf32,
    InvalidCodePoint,

    // Vocabulary
    VocabularyNotLoaded,
    VocabularyCorrupted,
    TokenNotFound,
    DuplicateToken,

    // Trainer
    DatasetEmpty,
    DatasetCorrupted,
    InvalidConfiguration,
    TrainingInterrupted,

    // Memory
    OutOfMemory,

    // Internal
    InternalError
};

constexpr std::string_view ToString(ErrorCode error) noexcept
{
    switch (error)
    {
        case ErrorCode::None: return "None";

        case ErrorCode::Unknown: return "Unknown";
        case ErrorCode::InvalidArgument: return "InvalidArgument";
        case ErrorCode::InvalidState: return "InvalidState";
        case ErrorCode::NotImplemented: return "NotImplemented";

        case ErrorCode::FileNotFound: return "FileNotFound";
        case ErrorCode::FileOpenFailed: return "FileOpenFailed";
        case ErrorCode::FileReadFailed: return "FileReadFailed";
        case ErrorCode::FileWriteFailed: return "FileWriteFailed";
        case ErrorCode::DirectoryNotFound: return "DirectoryNotFound";

        case ErrorCode::InvalidUtf8: return "InvalidUtf8";
        case ErrorCode::InvalidUtf16: return "InvalidUtf16";
        case ErrorCode::InvalidUtf32: return "InvalidUtf32";
        case ErrorCode::InvalidCodePoint: return "InvalidCodePoint";

        case ErrorCode::VocabularyNotLoaded: return "VocabularyNotLoaded";
        case ErrorCode::VocabularyCorrupted: return "VocabularyCorrupted";
        case ErrorCode::TokenNotFound: return "TokenNotFound";
        case ErrorCode::DuplicateToken: return "DuplicateToken";

        case ErrorCode::DatasetEmpty: return "DatasetEmpty";
        case ErrorCode::DatasetCorrupted: return "DatasetCorrupted";
        case ErrorCode::InvalidConfiguration: return "InvalidConfiguration";
        case ErrorCode::TrainingInterrupted: return "TrainingInterrupted";

        case ErrorCode::OutOfMemory: return "OutOfMemory";

        case ErrorCode::InternalError: return "InternalError";
    }

    return "Unknown";
}

} // namespace qualix
