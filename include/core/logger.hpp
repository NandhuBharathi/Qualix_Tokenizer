#pragma once

#include <fstream>
#include <mutex>
#include <string_view>

namespace qualix
{

enum class LogLevel
{
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

class Logger
{
public:
    explicit Logger(const std::string& file = "log.txt");

    ~Logger();

    void Log(LogLevel level, std::string_view message);

    void Trace(std::string_view message);
    void Debug(std::string_view message);
    void Info(std::string_view message);
    void Warning(std::string_view message);
    void Error(std::string_view message);
    void Fatal(std::string_view message);

private:
    std::ofstream file_;
    std::mutex mutex_;
};

} // namespace qualix
