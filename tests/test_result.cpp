#include <string>

#include "core/result.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::test;

int main()
{
    Result<int> number(42);

    Expect(number.Ok(), "Result<int>::Ok()");
    Expect(!number.Failed(), "Result<int>::Failed()");
    Expect(number.Value() == 42, "Result<int>::Value()");
    Expect(number.GetStatus().Code() == ErrorCode::None,
           "Result<int>::Status()");

    Result<std::string> text(std::string("Qualix"));

    Expect(text.Ok(), "Result<string>::Ok()");
    Expect(text.Value() == "Qualix", "Result<string>::Value()");

    Result<int> failed(Status::Failure(ErrorCode::FileNotFound));

    Expect(!failed.Ok(), "Failed Result::Ok()");
    Expect(failed.Failed(), "Failed Result::Failed()");
    Expect(failed.GetStatus().Code() == ErrorCode::FileNotFound,
           "Failed Result::Status()");
    Expect(std::string_view(failed.GetStatus().Message()) == "FileNotFound",
           "Failed Result::Message()");

    Result<int> mutable_value(10);

    mutable_value.Value() = 25;

    Expect(mutable_value.Value() == 25,
           "Mutable Result::Value()");

    return Summary();
}
