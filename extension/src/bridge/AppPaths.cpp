#include "bridge/AppPaths.h"

#include <cstdlib>

namespace presonus::studiolive::gpext::bridge
{

std::filesystem::path appDataDirectory()
{
    if (const char *appdata = std::getenv("APPDATA"))
    {
        return std::filesystem::path(appdata) / "PreSonusStudioLive";
    }
    return std::filesystem::temp_directory_path() / "PreSonusStudioLive";
}

} // namespace presonus::studiolive::gpext::bridge
