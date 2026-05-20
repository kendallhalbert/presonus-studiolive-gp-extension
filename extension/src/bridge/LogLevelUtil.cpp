#include "bridge/LogLevelUtil.h"

namespace presonus::studiolive::gpext::bridge
{

std::optional<LogLevel> parseLogLevel(std::string_view text)
{
    if (text == "none")
    {
        return LogLevel::None;
    }
    if (text == "error")
    {
        return LogLevel::Error;
    }
    if (text == "warn" || text == "warning")
    {
        return LogLevel::Warn;
    }
    if (text == "info")
    {
        return LogLevel::Info;
    }
    if (text == "debug")
    {
        return LogLevel::Debug;
    }
    return std::nullopt;
}

} // namespace presonus::studiolive::gpext::bridge
