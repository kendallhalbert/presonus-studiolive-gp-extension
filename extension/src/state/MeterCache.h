#pragma once

#include "protocol/LevlParser.h"

#include <cstdint>
#include <mutex>
#include <optional>

namespace presonus::studiolive::gpext::state
{

/// Thread-safe cache of the most recent UCNet meter levels (groupId + 1-based channel).
class MeterCache
{
  public:
    void apply(const protocol::LevlMessage &message);
    void clear();

    /// Returns linear 0..100 percent (raw uint16 / 655.35), or nullopt if unknown.
    std::optional<double> levelPercent(int groupId, int channel) const;

    bool hasData() const;

  private:
    mutable std::mutex mutex_;
    std::uint16_t levels_[protocol::kLevlGroupCount][protocol::kLevlChannelsPerGroup]{};
    bool populated_[protocol::kLevlGroupCount][protocol::kLevlChannelsPerGroup]{};
};

} // namespace presonus::studiolive::gpext::state
