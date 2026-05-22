#pragma once

#include "bridge/Logger.h"
#include "protocol/DiscoveryParser.h"

#include <chrono>
#include <vector>

namespace presonus::studiolive::gpext::transport
{

/// Listen for UCNet UDP discovery broadcasts on port 47809.
std::vector<protocol::DiscoveredMixer>
listenForDiscovery(std::chrono::milliseconds timeout, bridge::Logger &logger);

} // namespace presonus::studiolive::gpext::transport
