#include "core/status.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::test;

int main()
{
    Status ok;

    Expect(ok.Ok(), "Default Status::Ok()");
    Expect(!ok.Failed(), "Default Status::Failed()");
    Expect(ok.Code() == ErrorCode::None, "Default Status::Code()");
    Expect(std::string_view(ok.Message()) == "None", "Default Status::Message()");

    Status file_error(ErrorCode::FileNotFound);

    Expect(!file_error.Ok(), "FileError::Ok()");
    Expect(file_error.Failed(), "FileError::Failed()");
    Expect(file_error.Code() == ErrorCode::FileNotFound, "FileError::Code()");
    Expect(std::string_view(file_error.Message()) == "FileNotFound", "FileError::Message()");

    auto success = Status::Success();

    Expect(success.Ok(), "Status::Success()");
    Expect(success.Code() == ErrorCode::None, "Status::Success Code");

    auto failure = Status::Failure(ErrorCode::OutOfMemory);

    Expect(failure.Failed(), "Status::Failure()");
    Expect(failure.Code() == ErrorCode::OutOfMemory, "Status::Failure Code");
    Expect(std::string_view(failure.Message()) == "OutOfMemory",
           "Status::Failure Message");

    return Summary();
}
