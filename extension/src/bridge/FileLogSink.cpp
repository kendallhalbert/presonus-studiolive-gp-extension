#include "bridge/FileLogSink.h"

#include "bridge/AppPaths.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace presonus::studiolive::gpext::bridge
{

namespace
{

const char *levelName(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Error:
        return "ERROR";
    case LogLevel::Warn:
        return "WARN";
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Debug:
        return "DEBUG";
    default:
        return "NONE";
    }
}

std::string localTimestamp()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm localTime{};
#if defined(_WIN32)
    localtime_s(&localTime, &time);
#else
    localtime_r(&time, &localTime);
#endif
    std::ostringstream formatted;
    formatted << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return formatted.str();
}

} // namespace

std::filesystem::path FileLogSink::defaultLogPath()
{
    return appDataDirectory() / "extension.log";
}

FileLogSink::FileLogSink(std::filesystem::path path) : path_(std::move(path))
{
    std::error_code ec;
    std::filesystem::create_directories(path_.parent_path(), ec);
}

std::string FileLogSink::formatLine(LogLevel level, std::string_view message) const
{
    std::ostringstream line;
    line << localTimestamp() << " [" << levelName(level) << "] " << message;
    return line.str();
}

void FileLogSink::write(LogLevel level, std::string_view message)
{
    std::lock_guard lock(mutex_);
    std::ofstream out(path_, std::ios::app);
    if (!out)
    {
        return;
    }
    out << formatLine(level, message) << '\n';
    out.flush();
}

void FileLogSink::writeSessionBanner(std::string_view version)
{
    std::lock_guard lock(mutex_);
    std::ofstream out(path_, std::ios::app);
    if (!out)
    {
        return;
    }
    out << "\n--- PreSonus StudioLive " << version << " (" << localTimestamp()
        << ") log: " << path_.string() << " ---\n";
    out.flush();
}

} // namespace presonus::studiolive::gpext::bridge
