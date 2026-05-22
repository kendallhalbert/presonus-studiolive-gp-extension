#include "bridge/ConfigStore.h"

#include "bridge/AppPaths.h"

#include <fstream>
#include <sstream>

namespace presonus::studiolive::gpext::bridge
{

namespace
{

std::optional<std::string> readJsonStringField(const std::string &text, const std::string &field)
{
    const std::string key = '"' + field + '"';
    const auto pos = text.find(key);
    if (pos == std::string::npos)
    {
        return std::nullopt;
    }
    const auto colon = text.find(':', pos + key.size());
    if (colon == std::string::npos)
    {
        return std::nullopt;
    }
    const auto valueStart = text.find_first_not_of(" \t\r\n", colon + 1);
    if (valueStart == std::string::npos)
    {
        return std::nullopt;
    }
    if (text.compare(valueStart, 4, "null") == 0)
    {
        return std::nullopt;
    }
    if (text[valueStart] != '"')
    {
        return std::nullopt;
    }
    const auto quoteEnd = text.find('"', valueStart + 1);
    if (quoteEnd == std::string::npos)
    {
        return std::nullopt;
    }
    return text.substr(valueStart + 1, quoteEnd - valueStart - 1);
}

void writeJsonStringField(std::ostream &out, const char *field, const std::optional<std::string> &value)
{
    out << "  \"" << field << "\": ";
    if (value)
    {
        out << '"' << *value << '"';
    }
    else
    {
        out << "null";
    }
}

} // namespace

std::filesystem::path ConfigStore::configFilePath()
{
    return appDataDirectory() / "config.json";
}

bool ConfigStore::load()
{
    lastHost_.reset();
    lastSerial_.reset();
    lastMixerName_.reset();

    std::ifstream in(configFilePath());
    if (!in)
    {
        return false;
    }
    std::ostringstream contents;
    contents << in.rdbuf();
    const std::string text = contents.str();

    lastHost_ = readJsonStringField(text, "lastHost");
    lastSerial_ = readJsonStringField(text, "lastSerial");
    lastMixerName_ = readJsonStringField(text, "lastMixerName");
    return true;
}

bool ConfigStore::save() const
{
    const auto path = configFilePath();
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    std::ofstream out(path, std::ios::trunc);
    if (!out)
    {
        return false;
    }
    out << "{\n";
    writeJsonStringField(out, "lastHost", lastHost_);
    out << ",\n";
    writeJsonStringField(out, "lastSerial", lastSerial_);
    out << ",\n";
    writeJsonStringField(out, "lastMixerName", lastMixerName_);
    out << "\n}\n";
    return true;
}

void ConfigStore::setLastHost(std::string host) { lastHost_ = std::move(host); }

void ConfigStore::setLastSerial(std::string serial) { lastSerial_ = std::move(serial); }

void ConfigStore::setLastMixerName(std::string name) { lastMixerName_ = std::move(name); }

} // namespace presonus::studiolive::gpext::bridge
