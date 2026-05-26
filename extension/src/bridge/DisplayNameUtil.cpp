#include "bridge/DisplayNameUtil.h"

#include <cctype>

namespace presonus::studiolive::gpext::bridge
{

namespace
{

bool isDigit(char ch)
{
    return ch >= '0' && ch <= '9';
}

std::string stripKnownExtension(std::string_view name, std::string_view ext)
{
    if (name.size() >= ext.size() &&
        name.substr(name.size() - ext.size()) == ext)
    {
        name.remove_suffix(ext.size());
    }
    return std::string(name);
}

} // namespace

std::string presetFileDisplayName(std::string_view filename)
{
    if (filename.empty())
    {
        return {};
    }

    std::string name(filename);
    name = stripKnownExtension(name, ".scn");
    name = stripKnownExtension(name, ".proj");
    name = stripKnownExtension(name, ".cnfg");

    const auto dot = name.find('.');
    if (dot != std::string::npos && dot + 1 < name.size() && isDigit(name[0]))
    {
        bool allDigits = true;
        for (std::size_t i = 0; i < dot; ++i)
        {
            if (!isDigit(name[i]))
            {
                allDigits = false;
                break;
            }
        }
        if (allDigits)
        {
            name.erase(0, dot + 1);
        }
    }

    return name;
}

} // namespace presonus::studiolive::gpext::bridge
