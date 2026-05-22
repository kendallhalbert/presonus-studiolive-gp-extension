#pragma once

#include "bridge/GpHost.h"
#include "protocol/ChannelKeys.h"

#include <chrono>
#include <optional>
#include <string>
#include <unordered_map>

namespace presonus::studiolive::gpext::mixer
{
class MixerService;
}

namespace presonus::studiolive::gpext::bridge
{

enum class WidgetBindingKind
{
    LevelLinear,
    LevelDb,
    Mute,
    Solo,
};

enum class WidgetDirection : int
{
    MixerToWidget = 0,
    WidgetToMixer = 1,
    Both = 2,
};

/// GP-panel widget ↔ mixer parameter bindings with feedback-loop suppression.
class WidgetBindingRegistry
{
  public:
    bool bindLevelLinear(GpHost &host, const std::string &widget,
                         const protocol::ChannelTarget &target, WidgetDirection direction);
    bool bindLevelDb(GpHost &host, const std::string &widget,
                     const protocol::ChannelTarget &target, WidgetDirection direction);
    bool bindMute(GpHost &host, const std::string &widget,
                  const protocol::ChannelTarget &target, WidgetDirection direction);
    bool bindSolo(GpHost &host, const std::string &widget,
                  const protocol::ChannelTarget &target, WidgetDirection direction);

    bool bindLineLevelLinear(GpHost &host, const std::string &widget, int channel,
                             WidgetDirection direction);
    bool bindLineLevelDb(GpHost &host, const std::string &widget, int channel,
                         WidgetDirection direction);
    bool bindLineMute(GpHost &host, const std::string &widget, int channel,
                      WidgetDirection direction);
    bool bindLineSolo(GpHost &host, const std::string &widget, int channel,
                      WidgetDirection direction);

    bool unbind(GpHost &host, const std::string &widget);
    void unbindAll(GpHost &host);

    void onWidgetValueChanged(GpHost &host, mixer::MixerService &mixer,
                              const std::string &widget, double newValue);
    void pollMixerToWidgets(GpHost &host, mixer::MixerService &mixer);

  private:
    struct Binding
    {
        WidgetBindingKind kind{};
        WidgetDirection direction{WidgetDirection::Both};
        protocol::ChannelTarget target{};
    };

    struct Suppression
    {
        double value{0.0};
        std::chrono::steady_clock::time_point expires{};
    };

    bool bind(GpHost &host, const std::string &widget, Binding binding);
    bool allowsWidgetToMixer(WidgetDirection direction) const;
    bool allowsMixerToWidget(WidgetDirection direction) const;
    std::optional<double> readMixerWidgetValue(const Binding &binding,
                                               mixer::MixerService &mixer) const;
    void applyWidgetToMixer(const Binding &binding, mixer::MixerService &mixer,
                            double widgetValue) const;
    void pushWidgetValue(GpHost &host, const std::string &widget, double value);
    bool shouldSuppressEcho(const std::string &widget, double value) const;

    std::unordered_map<std::string, Binding> bindings_;
    std::unordered_map<std::string, double> lastPushed_;
    std::unordered_map<std::string, Suppression> suppressions_;
};

} // namespace presonus::studiolive::gpext::bridge
