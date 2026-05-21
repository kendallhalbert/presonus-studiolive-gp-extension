#pragma once

#include "bridge/Dispatcher.h"
#include "bridge/Logger.h"
#include "bridge/SongBindingTable.h"
#include "bridge/WidgetBindingRegistry.h"

#include <atomic>

namespace presonus::studiolive::gpext::mixer
{
class MixerService;
}

namespace presonus::studiolive::gpext::bridge
{

class ConfigStore;
class GpHost;

/// Process-wide context for GP-thread work. Set during LibMain::Initialization.
class ExtensionContext
{
  public:
    static ExtensionContext *instance();
    static void setInstance(ExtensionContext *ctx);

    ExtensionContext(GpHost &gpHost, Dispatcher &dispatcher, Logger &logger);

    GpHost &gpHost() { return gpHost_; }
    const GpHost &gpHost() const { return gpHost_; }
    Dispatcher &dispatcher() { return dispatcher_; }
    Logger &logger() { return logger_; }
    WidgetBindingRegistry &widgetBindings() { return widgetBindings_; }
    SongBindingTable &songBindings() { return songBindings_; }

    /// Run queued IO→GP tasks on the current (GP) thread.
    void drainGpTasks();

    /// Queue a mixer→widget poll on the GP thread (coalesced; safe from IO thread).
    void scheduleWidgetPoll();

    void setMixerService(mixer::MixerService *service) { mixer_ = service; }
    mixer::MixerService *mixerService() const { return mixer_; }

    void setConfigStore(ConfigStore *config) { config_ = config; }
    ConfigStore *configStore() const { return config_; }

  private:
    GpHost &gpHost_;
    Dispatcher &dispatcher_;
    Logger &logger_;
    WidgetBindingRegistry widgetBindings_;
    SongBindingTable songBindings_;
    mixer::MixerService *mixer_{nullptr};
    ConfigStore *config_{nullptr};
    std::atomic<bool> widgetPollScheduled_{false};
};

} // namespace presonus::studiolive::gpext::bridge
