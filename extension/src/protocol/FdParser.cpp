#include "protocol/FdParser.h"

namespace presonus::studiolive::gpext::protocol
{

namespace
{

std::optional<std::string> extractQuotedValue(std::string_view json, std::size_t &cursor,
                                              std::string_view key)
{
    const std::string keyToken = std::string("\"") + std::string(key) + "\"";
    const std::size_t keyPos = json.find(keyToken, cursor);
    if (keyPos == std::string_view::npos)
    {
        return std::nullopt;
    }

    std::size_t valueStart = keyPos + keyToken.size();
    while (valueStart < json.size() && (json[valueStart] == ':' || json[valueStart] == ' '))
    {
        ++valueStart;
    }
    if (valueStart >= json.size() || json[valueStart] != '"')
    {
        return std::nullopt;
    }
    ++valueStart;
    std::string value;
    while (valueStart < json.size())
    {
        const char ch = json[valueStart++];
        if (ch == '\\' && valueStart < json.size())
        {
            value.push_back(json[valueStart++]);
            continue;
        }
        if (ch == '"')
        {
            cursor = valueStart;
            return value;
        }
        value.push_back(ch);
    }
    return std::nullopt;
}

} // namespace

std::optional<std::vector<FdFileEntry>> parseFdFileList(std::string_view json)
{
    if (json.find("\"files\"") == std::string_view::npos)
    {
        return std::nullopt;
    }

    std::vector<FdFileEntry> entries;
    std::size_t cursor = 0;
    while (cursor < json.size())
    {
        const auto name = extractQuotedValue(json, cursor, "name");
        if (!name)
        {
            break;
        }
        FdFileEntry entry{.name = *name};
        const auto title = extractQuotedValue(json, cursor, "title");
        if (title)
        {
            entry.title = *title;
        }
        entries.push_back(std::move(entry));
    }

    if (entries.empty())
    {
        return std::nullopt;
    }
    return entries;
}

} // namespace presonus::studiolive::gpext::protocol
