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

} // namespace presonus::studiolive::gpext::protocol
