#pragma once

#include "bridge/Logger.h"

#include <optional>
#include <string_view>

namespace presonus::studiolive::gpext::bridge
{

std::optional<LogLevel> parseLogLevel(std::string_view text);

} // namespace presonus::studiolive::gpext::bridge
