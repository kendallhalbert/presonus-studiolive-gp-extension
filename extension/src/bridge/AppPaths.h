#pragma once

#include <filesystem>

namespace presonus::studiolive::gpext::bridge
{

/// `%APPDATA%\\PreSonusStudioLive` (or temp fallback when APPDATA is unset).
std::filesystem::path appDataDirectory();

} // namespace presonus::studiolive::gpext::bridge
