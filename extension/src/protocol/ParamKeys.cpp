#include "protocol/ParamKeys.h"

namespace presonus::studiolive::gpext::protocol
{

namespace
{

std::string lineChannelKey(int channel, std::string_view suffix)
{
    return std::string("line/ch") + std::to_string(channel) + "/" + std::string(suffix);
}

} // namespace

std::string lineChannelMuteKey(int channel) { return lineChannelKey(channel, "mute"); }

std::string lineChannelVolumeKey(int channel) { return lineChannelKey(channel, "volume"); }

std::string lineChannelLevelKey(int channel) { return lineChannelKey(channel, "level"); }

std::string lineChannelSoloKey(int channel) { return lineChannelKey(channel, "solo"); }

std::string lineChannelPanKey(int channel) { return lineChannelKey(channel, "pan"); }

std::string lineChannelColorKey(int channel) { return lineChannelKey(channel, "color"); }

} // namespace presonus::studiolive::gpext::protocol
