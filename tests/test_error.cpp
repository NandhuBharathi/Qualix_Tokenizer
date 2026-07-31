#include "core/error.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::test;

int main()
{
    Expect(ToString(ErrorCode::None) == "None", "ErrorCode::None");

    Expect(ToString(ErrorCode::Unknown) == "Unknown", "ErrorCode::Unknown");
    Expect(ToString(ErrorCode::InvalidArgument) == "InvalidArgument", "ErrorCode::InvalidArgument");
    Expect(ToString(ErrorCode::InvalidState) == "InvalidState", "ErrorCode::InvalidState");
    Expect(ToString(ErrorCode::NotImplemented) == "NotImplemented", "ErrorCode::NotImplemented");

    Expect(ToString(ErrorCode::FileNotFound) == "FileNotFound", "ErrorCode::FileNotFound");
    Expect(ToString(ErrorCode::FileOpenFailed) == "FileOpenFailed", "ErrorCode::FileOpenFailed");
    Expect(ToString(ErrorCode::FileReadFailed) == "FileReadFailed", "ErrorCode::FileReadFailed");
    Expect(ToString(ErrorCode::FileWriteFailed) == "FileWriteFailed", "ErrorCode::FileWriteFailed");
    Expect(ToString(ErrorCode::DirectoryNotFound) == "DirectoryNotFound", "ErrorCode::DirectoryNotFound");

    Expect(ToString(ErrorCode::InvalidUtf8) == "InvalidUtf8", "ErrorCode::InvalidUtf8");
    Expect(ToString(ErrorCode::InvalidUtf16) == "InvalidUtf16", "ErrorCode::InvalidUtf16");
    Expect(ToString(ErrorCode::InvalidUtf32) == "InvalidUtf32", "ErrorCode::InvalidUtf32");
    Expect(ToString(ErrorCode::InvalidCodePoint) == "InvalidCodePoint", "ErrorCode::InvalidCodePoint");

    Expect(ToString(ErrorCode::VocabularyNotLoaded) == "VocabularyNotLoaded", "ErrorCode::VocabularyNotLoaded");
    Expect(ToString(ErrorCode::VocabularyCorrupted) == "VocabularyCorrupted", "ErrorCode::VocabularyCorrupted");
    Expect(ToString(ErrorCode::TokenNotFound) == "TokenNotFound", "ErrorCode::TokenNotFound");
    Expect(ToString(ErrorCode::DuplicateToken) == "DuplicateToken", "ErrorCode::DuplicateToken");

    Expect(ToString(ErrorCode::DatasetEmpty) == "DatasetEmpty", "ErrorCode::DatasetEmpty");
    Expect(ToString(ErrorCode::DatasetCorrupted) == "DatasetCorrupted", "ErrorCode::DatasetCorrupted");
    Expect(ToString(ErrorCode::InvalidConfiguration) == "InvalidConfiguration", "ErrorCode::InvalidConfiguration");
    Expect(ToString(ErrorCode::TrainingInterrupted) == "TrainingInterrupted", "ErrorCode::TrainingInterrupted");

    Expect(ToString(ErrorCode::OutOfMemory) == "OutOfMemory", "ErrorCode::OutOfMemory");
    Expect(ToString(ErrorCode::InternalError) == "InternalError", "ErrorCode::InternalError");

    return Summary();
}
