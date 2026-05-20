#include "bridge/ConfigStore.h"

#include "bridge/AppPaths.h"

#include <fstream>
#include <sstream>

namespace presonus::studiolive::gpext::bridge
{

std::filesystem::path ConfigStore::configFilePath()
{
    return appDataDirectory() / "config.json";
}

bool ConfigStore::load()
{
    lastHost_.reset();
    std::ifstream in(configFilePath());
    if (!in)
    {
        return false;
    }
    std::ostringstream contents;
    contents << in.rdbuf();
    const std::string text = contents.str();
    const std::string key = "\"lastHost\"";
    const auto pos = text.find(key);
    if (pos == std::string::npos)
    {
        return true;
    }
    const auto colon = text.find(':', pos + key.size());
    const auto quoteStart = text.find('"', colon == std::string::npos ? pos : colon);
    if (quoteStart == std::string::npos)
    {
        return true;
    }
    const auto quoteEnd = text.find('"', quoteStart + 1);
    if (quoteEnd == std::string::npos || quoteEnd <= quoteStart + 1)
    {
        return true;
    }
    lastHost_ = text.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
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
    out << "{\n  \"lastHost\": ";
    if (lastHost_)
    {
        out << '"' << *lastHost_ << '"';
    }
    else
    {
        out << "null";
    }
    out << "\n}\n";
    return true;
}

void ConfigStore::setLastHost(std::string host) { lastHost_ = std::move(host); }

} // namespace presonus::studiolive::gpext::bridge
