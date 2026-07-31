#include "core/logger.hpp"

#include <iostream>

namespace qualix
{

Logger::Logger(const std::string& file)
{
    file_.open(file, std::ios::app);
}

Logger::~Logger()
{
    if (file_.is_open())
    {
        file_.close();
    }
}

void Logger::Log(LogLevel, std::string_view message)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (file_.is_open())
    {
        file_ << message << '\n';
    }
}

void Logger::Trace(std::string_view message)   { Log(LogLevel::Trace, message); }
void Logger::Debug(std::string_view message)   { Log(LogLevel::Debug, message); }
void Logger::Info(std::string_view message)    { Log(LogLevel::Info, message); }
void Logger::Warning(std::string_view message) { Log(LogLevel::Warning, message); }
void Logger::Error(std::string_view message)   { Log(LogLevel::Error, message); }
void Logger::Fatal(std::string_view message)   { Log(LogLevel::Fatal, message); }

} // namespace qualix
