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

bool isLineChannelValid(int channel) { return channel >= 1; }

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
    if (!isLineChannelValid(binding.channel) || !host.widgetExists(widget))
    {
        return false;
    }

    if (allowsWidgetToMixer(binding.direction))
    {
        host.listenForWidget(widget, true);
    }

    bindings_[widget] = binding;

    if (allowsMixerToWidget(binding.direction))
    {
        lastPushed_.erase(widget);
    }

    return true;
}

bool WidgetBindingRegistry::bindLineLevelLinear(GpHost &host, const std::string &widget,
                                                int channel, WidgetDirection direction)
{
    return bind(host, widget,
                Binding{.kind = WidgetBindingKind::LineLevelLinear,
                        .direction = direction,
                        .channel = channel});
}

bool WidgetBindingRegistry::bindLineLevelDb(GpHost &host, const std::string &widget,
                                          int channel, WidgetDirection direction)
{
    return bind(host, widget,
                Binding{.kind = WidgetBindingKind::LineLevelDb,
                        .direction = direction,
                        .channel = channel});
}

bool WidgetBindingRegistry::bindLineMute(GpHost &host, const std::string &widget, int channel,
                                       WidgetDirection direction)
{
    return bind(host, widget,
                Binding{.kind = WidgetBindingKind::LineMute,
                        .direction = direction,
                        .channel = channel});
}

bool WidgetBindingRegistry::bindLineSolo(GpHost &host, const std::string &widget, int channel,
                                         WidgetDirection direction)
{
    return bind(host, widget,
                Binding{.kind = WidgetBindingKind::LineSolo,
                        .direction = direction,
                        .channel = channel});
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
    case WidgetBindingKind::LineLevelLinear:
        if (const auto level = mixer.getLineLevelLinear(binding.channel))
        {
            return *level / 100.0;
        }
        return std::nullopt;
    case WidgetBindingKind::LineLevelDb:
        if (const auto db = mixer.getLineLevelDb(binding.channel))
        {
            return (*db + 84.0) / 94.0;
        }
        return std::nullopt;
    case WidgetBindingKind::LineMute:
        if (const auto muted = mixer.getLineMute(binding.channel))
        {
            return *muted ? 1.0 : 0.0;
        }
        return std::nullopt;
    case WidgetBindingKind::LineSolo:
        if (const auto soloed = mixer.getLineSolo(binding.channel))
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
    case WidgetBindingKind::LineLevelLinear:
        mixer.setLineLevelLinear(binding.channel, widgetValue * 100.0);
        break;
    case WidgetBindingKind::LineLevelDb:
        mixer.setLineLevelDb(binding.channel, -84.0 + widgetValue * 94.0);
        break;
    case WidgetBindingKind::LineMute:
        mixer.setLineMute(binding.channel, widgetValue >= 0.5);
        break;
    case WidgetBindingKind::LineSolo:
        mixer.setLineSolo(binding.channel, widgetValue >= 0.5);
        break;
    }
}

void WidgetBindingRegistry::onWidgetValueChanged(GpHost &host, mixer::MixerService &mixer,
                                                 const std::string &widget, double newValue)
{
    (void)host;
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
    lastPushed_[widget] = newValue;
}

void WidgetBindingRegistry::pollMixerToWidgets(GpHost &host, mixer::MixerService &mixer)
{
    for (const auto &[widget, binding] : bindings_)
    {
        if (!allowsMixerToWidget(binding.direction))
        {
            continue;
        }

        const auto widgetValue = readMixerWidgetValue(binding, mixer);
        if (!widgetValue.has_value())
        {
            continue;
        }

        const auto last = lastPushed_.find(widget);
        if (last != lastPushed_.end() &&
            std::abs(last->second - *widgetValue) <= kWidgetEpsilon)
        {
            continue;
        }

        pushWidgetValue(host, widget, *widgetValue);
    }
}

} // namespace presonus::studiolive::gpext::bridge
