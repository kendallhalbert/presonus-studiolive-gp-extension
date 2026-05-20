#pragma once

#include "protocol/SessionPacketDecoder.h"
#include "protocol/ZlibState.h"

#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace presonus::studiolive::gpext::state
{

/// Flat key/value cache for mixer parameters (PV/PS/PC keys and MS fader levels).
class KvCache
{
  public:
    void clear();

    void apply(const protocol::SessionPacket &packet);

    void setBool(std::string key, bool value);
    void setFloat(std::string key, double value);
    void setString(std::string key, std::string value);

    std::optional<bool> boolKey(std::string_view key) const;
    std::optional<double> doubleKey(std::string_view key) const;
    std::optional<std::string> stringKey(std::string_view key) const;

  private:
    void applyPv(const protocol::PvMessage &message);
    void applyPs(const protocol::PsMessage &message);
    void applyPc(const protocol::PcMessage &message);
    void applyMs(const protocol::MsMessage &message);
    void importZlibState(const protocol::ZlibStateNode &root, std::string prefix);

    static std::string msLevelKey(std::string_view channelType, int channel1Based);

    mutable std::mutex mutex_;
    std::unordered_map<std::string, protocol::StateValue> values_;
};

} // namespace presonus::studiolive::gpext::state
