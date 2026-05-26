#pragma once

#include <string>
#include <string_view>

namespace presonus::studiolive::gpext::bridge
{

/// Strip optional leading `NN.` and trailing `.scn` / `.proj` from a UCNet preset filename.
std::string presetFileDisplayName(std::string_view filename);

} // namespace presonus::studiolive::gpext::bridge
