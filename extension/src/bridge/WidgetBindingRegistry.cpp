#include "bridge/WidgetBindingRegistry.h"

#include "mixer/MixerService.h"
#include "protocol/ValueUtil.h"

#include <cmath>

namespace presonus::studiolive::gpext::bridge
{

namespace
{

constexpr auto kEchoSuppressWindow = std::chrono::milliseconds(50);
constexpr double kWidgetEpsilon = 0.0005;

std::optional<protocol::ChannelTarget> lineTarget(int channel)
{
    return protocol::parseChannelTarget("LINE", channel, "", 0);
}

} // namespace

bool WidgetBindingRegistry::allowsWidgetToMixer(WidgetDirection direction) const
{
    return direction == WidgetDirection::WidgetToMixer ||
           direction == WidgetDirection::Both;
}

bool WidgetBindingRegistry::allowsMixerToWidget(WidgetDirection direction) const
{
    return direction == WidgetDirection::MixerToWidget ||
           direction == WidgetDirection::Both;
}

bool WidgetBindingRegistry::bind(GpHost &host, const std::string &widget, Binding binding)
{
    if (binding.target.channel < 1)
    {
        return false;
    }

    if (binding.kind == WidgetBindingKind::Solo &&
        binding.target.mixKind != protocol::MixKind::Main)
    {
        return false;
    }

    const bool exists = host.widgetExists(widget);
    if (!exists && allowsMixerToWidget(binding.direction))
    {
        return false;
    }

    if (allowsWidgetToMixer(binding.direction))
    {
        if (!host.listenForWidget(widget, true))
        {
            return false;
        }
    }
    else if (!exists)
    {
        return false;
    }

    bindings_[widget] = binding;

    if (allowsMixerToWidget(binding.direction))
    {
        lastPushed_.erase(widget);
    }

    return true;
}

bool WidgetBindingRegistry::bindLevelLinear(GpHost &host, const std::string &widget,
                                            const protocol::ChannelTarget &target,
                                            WidgetDirection direction)
{
    return bind(host, widget,
                Binding{.kind = WidgetBindingKind::LevelLinear,
                        .direction = direction,
                        .target = target});
}

bool WidgetBindingRegistry::bindLevelDb(GpHost &host, const std::string &widget,
                                        const protocol::ChannelTarget &target,
                                        WidgetDirection direction)
{
    return bind(host, widget,
                Binding{.kind = WidgetBindingKind::LevelDb,
                        .direction = direction,
                        .target = target});
}

bool WidgetBindingRegistry::bindMute(GpHost &host, const std::string &widget,
                                     const protocol::ChannelTarget &target,
                                     WidgetDirection direction)
{
    return bind(host, widget,
                Binding{.kind = WidgetBindingKind::Mute,
                        .direction = direction,
                        .target = target});
}

bool WidgetBindingRegistry::bindSolo(GpHost &host, const std::string &widget,
                                     const protocol::ChannelTarget &target,
                                     WidgetDirection direction)
{
    return bind(host, widget,
                Binding{.kind = WidgetBindingKind::Solo,
                        .direction = direction,
                        .target = target});
}

bool WidgetBindingRegistry::bindLineLevelLinear(GpHost &host, const std::string &widget,
                                                int channel, WidgetDirection direction)
{
    const auto target = lineTarget(channel);
    return target.has_value() &&
           bindLevelLinear(host, widget, *target, direction);
}

bool WidgetBindingRegistry::bindLineLevelDb(GpHost &host, const std::string &widget,
                                            int channel, WidgetDirection direction)
{
    const auto target = lineTarget(channel);
    return target.has_value() && bindLevelDb(host, widget, *target, direction);
}

bool WidgetBindingRegistry::bindLineMute(GpHost &host, const std::string &widget,
                                         int channel, WidgetDirection direction)
{
    const auto target = lineTarget(channel);
    return target.has_value() && bindMute(host, widget, *target, direction);
}

bool WidgetBindingRegistry::bindLineSolo(GpHost &host, const std::string &widget,
                                         int channel, WidgetDirection direction)
{
    const auto target = lineTarget(channel);
    return target.has_value() && bindSolo(host, widget, *target, direction);
}

bool WidgetBindingRegistry::unbind(GpHost &host, const std::string &widget)
{
    const auto it = bindings_.find(widget);
    if (it == bindings_.end())
    {
        return false;
    }

    if (allowsWidgetToMixer(it->second.direction))
    {
        host.listenForWidget(widget, false);
    }

    bindings_.erase(it);
    lastPushed_.erase(widget);
    suppressions_.erase(widget);
    return true;
}

void WidgetBindingRegistry::unbindAll(GpHost &host)
{
    for (const auto &[widget, binding] : bindings_)
    {
        if (allowsWidgetToMixer(binding.direction))
        {
            host.listenForWidget(widget, false);
        }
    }
    bindings_.clear();
    lastPushed_.clear();
    suppressions_.clear();
}

bool WidgetBindingRegistry::shouldSuppressEcho(const std::string &widget,
                                               double value) const
{
    const auto it = suppressions_.find(widget);
    if (it == suppressions_.end())
    {
        return false;
    }

    if (std::chrono::steady_clock::now() > it->second.expires)
    {
        return false;
    }

    return std::abs(it->second.value - value) <= kWidgetEpsilon;
}

void WidgetBindingRegistry::pushWidgetValue(GpHost &host, const std::string &widget,
                                            double value)
{
    suppressions_[widget] = Suppression{
        .value = value,
        .expires = std::chrono::steady_clock::now() + kEchoSuppressWindow,
    };
    host.setWidgetValue(widget, value);
    lastPushed_[widget] = value;
}

std::optional<double> WidgetBindingRegistry::readMixerWidgetValue(
    const Binding &binding, mixer::MixerService &mixer) const
{
    switch (binding.kind)
    {
    case WidgetBindingKind::LevelLinear:
        if (const auto level = mixer.getChannelLevelLinear(binding.target))
        {
            return *level / 100.0;
        }
        return std::nullopt;
    case WidgetBindingKind::LevelDb:
        if (const auto db = mixer.getChannelLevelDb(binding.target))
        {
            return (*db + 84.0) / 94.0;
        }
        return std::nullopt;
    case WidgetBindingKind::Mute:
        if (const auto muted = mixer.getChannelMute(binding.target))
        {
            return *muted ? 1.0 : 0.0;
        }
        return std::nullopt;
    case WidgetBindingKind::Solo:
        if (const auto soloed = mixer.getChannelSolo(binding.target))
        {
            return *soloed ? 1.0 : 0.0;
        }
        return std::nullopt;
    }
    return std::nullopt;
}

void WidgetBindingRegistry::applyWidgetToMixer(const Binding &binding,
                                               mixer::MixerService &mixer,
                                               double widgetValue) const
{
    switch (binding.kind)
    {
    case WidgetBindingKind::LevelLinear:
        mixer.setChannelLevelLinear(binding.target, widgetValue * 100.0);
        break;
    case WidgetBindingKind::LevelDb:
        mixer.setChannelLevelDb(binding.target, -84.0 + widgetValue * 94.0);
        break;
    case WidgetBindingKind::Mute:
        mixer.setChannelMute(binding.target, widgetValue >= 0.5);
        break;
    case WidgetBindingKind::Solo:
        mixer.setChannelSolo(binding.target, widgetValue >= 0.5);
        break;
    }
}

void WidgetBindingRegistry::onWidgetValueChanged(GpHost &host, mixer::MixerService &mixer,
                                                 const std::string &widget, double newValue)
{
    const auto it = bindings_.find(widget);
    if (it == bindings_.end() || !allowsWidgetToMixer(it->second.direction))
    {
        return;
    }

    if (shouldSuppressEcho(widget, newValue))
    {
        return;
    }

    applyWidgetToMixer(it->second, mixer, newValue);
}

void WidgetBindingRegistry::pollMixerToWidgets(GpHost &host, mixer::MixerService &mixer)
{
    for (const auto &[widget, binding] : bindings_)
    {
        if (!allowsMixerToWidget(binding.direction))
        {
            continue;
        }

        const auto value = readMixerWidgetValue(binding, mixer);
        if (!value.has_value())
        {
            continue;
        }

        const auto last = lastPushed_.find(widget);
        if (last != lastPushed_.end() &&
            std::abs(last->second - *value) <= kWidgetEpsilon)
        {
            continue;
        }

        pushWidgetValue(host, widget, *value);
    }
}

} // namespace presonus::studiolive::gpext::bridge
