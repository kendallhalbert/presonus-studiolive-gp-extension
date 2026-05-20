#pragma once

#include "bridge/Dispatcher.h"
#include "bridge/Logger.h"

namespace presonus::studiolive::gpext::bridge
{

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

  private:
    GpHost &gpHost_;
    Dispatcher &dispatcher_;
    Logger &logger_;
};

} // namespace presonus::studiolive::gpext::bridge
