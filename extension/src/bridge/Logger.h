#pragma once

#include <functional>
#include <mutex>
#include <string>
#include <string_view>

namespace presonus::studiolive::gpext::bridge
{

enum class LogLevel
{
    None = 0,
    Error,
    Warn,
    Info,
    Debug,
};

/// Thread-safe logger with a configurable minimum level. Production code
/// typically wires \p sink to forward into GP via GpHost::consoleLog.
class Logger
{
  public:
    using Sink = std::function<void(LogLevel level, std::string_view message)>;

    void setMinLevel(LogLevel level);
    LogLevel minLevel() const;

    void setSink(Sink sink);

    void log(LogLevel level, std::string_view message);
    void error(std::string_view message) { log(LogLevel::Error, message); }
    void warn(std::string_view message) { log(LogLevel::Warn, message); }
    void info(std::string_view message) { log(LogLevel::Info, message); }
    void debug(std::string_view message) { log(LogLevel::Debug, message); }

  private:
    mutable std::mutex mutex_;
    LogLevel minLevel_{LogLevel::Info};
    Sink sink_;
};

} // namespace presonus::studiolive::gpext::bridge
