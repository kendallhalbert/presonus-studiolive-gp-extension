#pragma once

#include "bridge/Logger.h"

#include <filesystem>
#include <mutex>
#include <string_view>

namespace presonus::studiolive::gpext::bridge
{

/// Append-only debug log under the app data directory (default: `extension.log`).
class FileLogSink
{
  public:
    static std::filesystem::path defaultLogPath();

    explicit FileLogSink(std::filesystem::path path = defaultLogPath());

    void write(LogLevel level, std::string_view message);

    /// Write a session banner (extension version, path). Called once at startup.
    void writeSessionBanner(std::string_view version);

    const std::filesystem::path &path() const { return path_; }

  private:
    std::string formatLine(LogLevel level, std::string_view message) const;

    std::filesystem::path path_;
    mutable std::mutex mutex_;
};

} // namespace presonus::studiolive::gpext::bridge
