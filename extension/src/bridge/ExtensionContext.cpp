#include "bridge/ExtensionContext.h"

#include "bridge/GpHost.h"

namespace presonus::studiolive::gpext::bridge
{

namespace
{
ExtensionContext *gInstance = nullptr;
}

ExtensionContext *ExtensionContext::instance() { return gInstance; }

void ExtensionContext::setInstance(ExtensionContext *ctx) { gInstance = ctx; }

ExtensionContext::ExtensionContext(GpHost &gpHost, Dispatcher &dispatcher, Logger &logger)
    : gpHost_(gpHost), dispatcher_(dispatcher), logger_(logger)
{
}

void ExtensionContext::drainGpTasks() { dispatcher_.drain(); }

} // namespace presonus::studiolive::gpext::bridge
