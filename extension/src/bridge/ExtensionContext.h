#pragma once

#include "bridge/Dispatcher.h"
#include "bridge/Logger.h"

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

    /// Run queued IO→GP tasks on the current (GP) thread.
    void drainGpTasks();

    void setMixerService(mixer::MixerService *service) { mixer_ = service; }
    mixer::MixerService *mixerService() const { return mixer_; }

    void setConfigStore(ConfigStore *config) { config_ = config; }
    ConfigStore *configStore() const { return config_; }

  private:
    GpHost &gpHost_;
    Dispatcher &dispatcher_;
    Logger &logger_;
    mixer::MixerService *mixer_{nullptr};
    ConfigStore *config_{nullptr};
};

} // namespace presonus::studiolive::gpext::bridge
