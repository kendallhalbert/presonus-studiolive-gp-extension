#include "state/MeterCache.h"

namespace presonus::studiolive::gpext::state
{

void MeterCache::apply(const protocol::LevlMessage &message)
{
    std::lock_guard lock(mutex_);
    for (const auto &[groupId, levels] : message.levelsByGroup)
    {
        if (groupId < 0 || groupId >= protocol::kLevlGroupCount)
        {
            continue;
        }

        const int count =
            std::min(static_cast<int>(levels.size()), protocol::kLevlChannelsPerGroup);
        for (int channel = 0; channel < count; ++channel)
        {
            levels_[groupId][channel] = levels[static_cast<std::size_t>(channel)];
            populated_[groupId][channel] = true;
        }
    }
}

void MeterCache::clear()
{
    std::lock_guard lock(mutex_);
    for (auto &group : levels_)
    {
        for (auto &value : group)
        {
            value = 0;
        }
    }
    for (auto &group : populated_)
    {
        for (auto &set : group)
        {
            set = false;
        }
    }
}

std::optional<double> MeterCache::levelPercent(const int groupId, const int channel) const
{
    if (groupId < 0 || groupId >= protocol::kLevlGroupCount || channel < 1 ||
        channel > protocol::kLevlChannelsPerGroup)
    {
        return std::nullopt;
    }

    std::lock_guard lock(mutex_);
    const int index = channel - 1;
    if (!populated_[groupId][index])
    {
        return std::nullopt;
    }

    return static_cast<double>(levels_[groupId][index]) / 655.35;
}

bool MeterCache::hasData() const
{
    std::lock_guard lock(mutex_);
    for (const auto &group : populated_)
    {
        for (const bool set : group)
        {
            if (set)
            {
                return true;
            }
        }
    }
    return false;
}

} // namespace presonus::studiolive::gpext::state
