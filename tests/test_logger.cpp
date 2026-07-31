#include <filesystem>

#include "core/logger.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::test;

int main()
{
    constexpr const char* kLogFile = "test_logger.log";

    std::filesystem::remove(kLogFile);

    {
        Logger logger(kLogFile);

        logger.Trace("Trace message");
        logger.Debug("Debug message");
        logger.Info("Info message");
        logger.Warning("Warning message");
        logger.Error("Error message");
        logger.Fatal("Fatal message");

        Expect(true, "Logger methods executed");
    }

    Expect(std::filesystem::exists(kLogFile),
           "Log file created");

    Expect(std::filesystem::file_size(kLogFile) > 0,
           "Log file not empty");

    std::filesystem::remove(kLogFile);

    return Summary();
}
