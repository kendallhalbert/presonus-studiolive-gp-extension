#pragma once

#include <string>
#include <string_view>

namespace presonus::studiolive::gpext::bridge
{

/// Returns true when `type` is a supported LINE input channel selector.
bool isLineChannelType(std::string_view type);

} // namespace presonus::studiolive::gpext::bridge
