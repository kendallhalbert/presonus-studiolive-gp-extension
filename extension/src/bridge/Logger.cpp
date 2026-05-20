#include "bridge/Logger.h"

namespace presonus::studiolive::gpext::bridge
{

void Logger::setMinLevel(LogLevel level)
{
    std::lock_guard lock(mutex_);
    minLevel_ = level;
}

LogLevel Logger::minLevel() const
{
    std::lock_guard lock(mutex_);
    return minLevel_;
}

void Logger::setSink(Sink sink)
{
    std::lock_guard lock(mutex_);
    sink_ = std::move(sink);
}

void Logger::log(LogLevel level, std::string_view message)
{
    Sink sinkCopy;
    LogLevel minLevelCopy;
    {
        std::lock_guard lock(mutex_);
        minLevelCopy = minLevel_;
        if (minLevelCopy == LogLevel::None ||
            static_cast<int>(level) > static_cast<int>(minLevelCopy))
        {
            return;
        }
        sinkCopy = sink_;
    }
    if (sinkCopy)
    {
        sinkCopy(level, message);
    }
}

} // namespace presonus::studiolive::gpext::bridge
