#pragma once

#include <string>

#include "core/types.hpp"

namespace qualix
{

struct Config
{
    // General
    bool verbose = false;
    bool enable_logging = true;

    // Threads
    u32 thread_count = 0;

    // Memory
    usize chunk_size = 16 * 1024 * 1024; // 16 MB

    // Logging
    std::string log_file = "log.txt";

    // Reports
    std::string report_file = "training_report.txt";
};

} // namespace qualix
