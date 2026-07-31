#include <string_view>

#include "core/config.hpp"
#include "test_framework.hpp"

using namespace qualix;
using namespace qualix::test;

int main()
{
    Config config;

    Expect(config.verbose == false,
           "Config::verbose");

    Expect(config.enable_logging == true,
           "Config::enable_logging");

    Expect(config.thread_count == 0,
           "Config::thread_count");

    Expect(config.chunk_size == 16 * 1024 * 1024,
           "Config::chunk_size");

    Expect(std::string_view(config.log_file) == "log.txt",
           "Config::log_file");

    Expect(std::string_view(config.report_file) == "training_report.txt",
           "Config::report_file");

    config.verbose = true;
    config.enable_logging = false;
    config.thread_count = 8;
    config.chunk_size = 32 * 1024 * 1024;
    config.log_file = "qualix.log";
    config.report_file = "report.txt";

    Expect(config.verbose == true,
           "Config modify verbose");

    Expect(config.enable_logging == false,
           "Config modify enable_logging");

    Expect(config.thread_count == 8,
           "Config modify thread_count");

    Expect(config.chunk_size == 32 * 1024 * 1024,
           "Config modify chunk_size");

    Expect(std::string_view(config.log_file) == "qualix.log",
           "Config modify log_file");

    Expect(std::string_view(config.report_file) == "report.txt",
           "Config modify report_file");

    return Summary();
}
