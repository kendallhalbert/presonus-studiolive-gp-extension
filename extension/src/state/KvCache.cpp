#include "state/KvCache.h"

#include "protocol/PvParser.h"

#include <algorithm>
#include <cctype>
#include <cstdint>

namespace presonus::studiolive::gpext::state
{

namespace
{

std::string toLower(std::string_view text)
{
    std::string out(text);
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

std::optional<bool> stateValueAsBool(const protocol::StateValue &value)
{
    if (std::holds_alternative<bool>(value))
    {
        return std::get<bool>(value);
    }
    if (std::holds_alternative<std::int64_t>(value))
    {
        return std::get<std::int64_t>(value) != 0;
    }
    if (std::holds_alternative<double>(value))
    {
        return std::get<double>(value) >= 0.5;
    }
    return std::nullopt;
}

std::optional<double> stateValueAsDouble(const protocol::StateValue &value)
{
    if (std::holds_alternative<double>(value))
    {
        return std::get<double>(value);
    }
    if (std::holds_alternative<std::int64_t>(value))
    {
        return static_cast<double>(std::get<std::int64_t>(value));
    }
    if (std::holds_alternative<bool>(value))
    {
        return std::get<bool>(value) ? 1.0 : 0.0;
    }
    return std::nullopt;
}

} // namespace

void KvCache::clear()
{
    std::lock_guard lock(mutex_);
    values_.clear();
}

void KvCache::apply(const protocol::SessionPacket &packet)
{
    if (const auto *pv = std::get_if<protocol::PvMessage>(&packet.payload))
    {
        applyPv(*pv);
        return;
    }
    if (const auto *ps = std::get_if<protocol::PsMessage>(&packet.payload))
    {
        applyPs(*ps);
        return;
    }
    if (const auto *pc = std::get_if<protocol::PcMessage>(&packet.payload))
    {
        applyPc(*pc);
        return;
    }
    if (const auto *ms = std::get_if<protocol::MsMessage>(&packet.payload))
    {
        applyMs(*ms);
        return;
    }
    if (const auto *zb = std::get_if<protocol::ZlibStateNode>(&packet.payload))
    {
        importZlibState(*zb, {});
    }
}

void KvCache::setBool(std::string key, bool value)
{
    std::lock_guard lock(mutex_);
    values_[std::move(key)] = value;
}

void KvCache::setFloat(std::string key, double value)
{
    std::lock_guard lock(mutex_);
    values_[std::move(key)] = value;
}

void KvCache::setString(std::string key, std::string value)
{
    std::lock_guard lock(mutex_);
    values_[std::move(key)] = std::move(value);
}

std::optional<bool> KvCache::boolKey(std::string_view key) const
{
    std::lock_guard lock(mutex_);
    const auto it = values_.find(std::string(key));
    if (it == values_.end())
    {
        return std::nullopt;
    }
    return stateValueAsBool(it->second);
}

std::optional<double> KvCache::doubleKey(std::string_view key) const
{
    std::lock_guard lock(mutex_);
    const auto it = values_.find(std::string(key));
    if (it == values_.end())
    {
        return std::nullopt;
    }
    return stateValueAsDouble(it->second);
}

std::optional<std::string> KvCache::stringKey(std::string_view key) const
{
    std::lock_guard lock(mutex_);
    const auto it = values_.find(std::string(key));
    if (it == values_.end())
    {
        return std::nullopt;
    }
    if (std::holds_alternative<std::string>(it->second))
    {
        return std::get<std::string>(it->second);
    }
    return std::nullopt;
}

void KvCache::applyPv(const protocol::PvMessage &message)
{
    if (const auto value = protocol::pvPayloadFloat(message.partB))
    {
        setFloat(message.key, static_cast<double>(*value));
    }
}

void KvCache::applyPs(const protocol::PsMessage &message)
{
    setString(message.key, message.value);
}

void KvCache::applyPc(const protocol::PcMessage &message)
{
    setString(message.key, message.valueHex);
}

std::string KvCache::msLevelKey(std::string_view channelType, int channel1Based)
{
    return toLower(channelType) + "/ch" + std::to_string(channel1Based) + "/level";
}

void KvCache::applyMs(const protocol::MsMessage &message)
{
    for (const auto &[channelType, levels] : message.levelsByChannelType)
    {
        for (std::size_t index = 0; index < levels.size(); ++index)
        {
            setFloat(msLevelKey(channelType, static_cast<int>(index) + 1), levels[index]);
        }
    }
}

int KvCache::countChannelsForWireType(std::string_view wireType) const
{
    std::lock_guard lock(mutex_);
    const std::string prefix = std::string(wireType) + "/ch";
    int maxChannel = 0;
    for (const auto &[key, _] : values_)
    {
        if (key.size() <= prefix.size() || key.compare(0, prefix.size(), prefix) != 0)
        {
            continue;
        }

        std::size_t pos = prefix.size();
        int channel = 0;
        while (pos < key.size() && std::isdigit(static_cast<unsigned char>(key[pos])))
        {
            channel = channel * 10 + (key[pos] - '0');
            ++pos;
        }

        if (pos > prefix.size() && key[pos] == '/')
        {
            maxChannel = std::max(maxChannel, channel);
        }
    }
    return maxChannel;
}

void KvCache::importZlibState(const protocol::ZlibStateNode &node, std::string prefix)
{
    if (node.value)
    {
        if (!prefix.empty())
        {
            std::lock_guard lock(mutex_);
            values_[prefix] = *node.value;
        }
    }

    for (const auto &[name, child] : node.children)
    {
        std::string path = prefix.empty() ? name : prefix + "/" + name;
        importZlibState(child, std::move(path));
    }
}

} // namespace presonus::studiolive::gpext::state
