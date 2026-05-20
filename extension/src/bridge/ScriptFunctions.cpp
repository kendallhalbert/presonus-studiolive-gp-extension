#include "bridge/ScriptFunctions.h"

#include "bridge/ConfigStore.h"
#include "bridge/ExtensionContext.h"
#include "mixer/MixerService.h"
#include "Version.h"

#include "gigperformer/sdk/imports.h"

#include <string>

namespace presonus::studiolive::gpext::bridge
{

namespace
{

void drainIfOnGpThread()
{
    if (ExtensionContext *ctx = ExtensionContext::instance())
    {
        ctx->drainGpTasks();
    }
}

mixer::MixerService *mixer()
{
    if (ExtensionContext *ctx = ExtensionContext::instance())
    {
        return ctx->mixerService();
    }
    return nullptr;
}

extern "C" void psl_Version(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    const std::string v = presonus::studiolive::gpext::versionString();
    GP_VM_PushString(vm, v.c_str());
}

extern "C" void psl_Connect(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    char hostBuffer[256] = {};
    GP_VM_PopString(vm, hostBuffer, static_cast<int>(sizeof(hostBuffer)));

    mixer::MixerService *const svc = mixer();
    const bool ok = svc != nullptr && svc->connect(hostBuffer);

    if (ok)
    {
        if (ExtensionContext *ctx = ExtensionContext::instance())
        {
            if (ConfigStore *config = ctx->configStore())
            {
                config->setLastHost(hostBuffer);
                config->save();
            }
        }
    }

    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_Disconnect(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    if (mixer::MixerService *const svc = mixer())
    {
        svc->disconnect();
    }
    (void)vm;
}

extern "C" void psl_IsConnected(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    mixer::MixerService *const svc = mixer();
    const bool ok = svc != nullptr && svc->isConnected();
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_SetLineMute(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int mutedFlag = GP_VM_PopInteger(vm);
    const int channel = GP_VM_PopInteger(vm);

    mixer::MixerService *const svc = mixer();
    const bool ok =
        svc != nullptr && svc->setLineMute(channel, mutedFlag != 0);
    GP_VM_PushBoolean(vm, ok);
}

ExternalAPI_GPScriptFunctionDefinition kScriptFunctions[] = {
    {
        "Version",
        "",
        "Returns String",
        "Returns the extension version string.",
        &psl_Version,
    },
    {
        "Connect",
        "host : String",
        "Returns Boolean",
        "Connect to a StudioLive mixer at the given IP/hostname (TCP port 53000).",
        &psl_Connect,
    },
    {
        "Disconnect",
        "",
        "",
        "Close the mixer TCP session.",
        &psl_Disconnect,
    },
    {
        "IsConnected",
        "",
        "Returns Boolean",
        "True when a mixer TCP session is active.",
        &psl_IsConnected,
    },
    {
        "SetLineMute",
        "channel : Integer, muted : Integer",
        "Returns Boolean",
        "Mute (muted=1) or unmute (muted=0) a 1-based input channel.",
        &psl_SetLineMute,
    },
};

} // namespace

int scriptFunctionCount()
{
    return static_cast<int>(sizeof(kScriptFunctions) / sizeof(kScriptFunctions[0]));
}

ExternalAPI_GPScriptFunctionDefinition *scriptFunctions() { return kScriptFunctions; }

} // namespace presonus::studiolive::gpext::bridge
