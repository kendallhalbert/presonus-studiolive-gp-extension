#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{

struct FdFileEntry
{
    std::string name;
    std::string title;
};

/// Parse `{ "files": [ { "name": "...", "title": "..." }, ... ] }` from reassembled FD JSON.
std::optional<std::vector<FdFileEntry>> parseFdFileList(std::string_view json);

/// True when `name` ends with `.scn` (case-insensitive). Project FD lists also contain `.cnfg` entries.
bool isFdSceneFile(std::string_view name);

/// Drop non-scene entries (e.g. project `.cnfg`) from a project scene list.
std::vector<FdFileEntry> filterFdSceneFiles(const std::vector<FdFileEntry> &entries);

/// True when `name` ends with `.ch` or `.channel` (case-insensitive).
bool isFdChannelPresetFile(std::string_view name);

/// Drop non-preset entries from channel preset lists (matches JS API + `.ch`/`.channel` suffix).
std::vector<FdFileEntry> filterFdChannelPresetFiles(const std::vector<FdFileEntry> &entries);

} // namespace presonus::studiolive::gpext::protocol
