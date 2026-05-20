#pragma once

#include <string>

namespace presonus::studiolive::gpext::protocol
{

/// UCNet parameter paths for 1-based LINE input channels.
std::string lineChannelMuteKey(int channel);
std::string lineChannelVolumeKey(int channel);
std::string lineChannelLevelKey(int channel);
std::string lineChannelSoloKey(int channel);
std::string lineChannelPanKey(int channel);
std::string lineChannelColorKey(int channel);

} // namespace presonus::studiolive::gpext::protocol
