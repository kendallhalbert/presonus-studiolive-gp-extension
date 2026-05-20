#include "bridge/ScriptFunctions.h"

#include "bridge/ConfigStore.h"
#include "bridge/ExtensionContext.h"
#include "bridge/FileLogSink.h"
#include "bridge/LogLevelUtil.h"
#include "mixer/MixerService.h"
#include "Version.h"

#include "gigperformer/sdk/imports.h"

#include <optional>
#include <string>
#include <string_view>

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

extern "C" void psl_SetLogLevel(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    char levelBuffer[32] = {};
    GP_VM_PopString(vm, levelBuffer, static_cast<int>(sizeof(levelBuffer)));

    ExtensionContext *const ctx = ExtensionContext::instance();
    if (!ctx)
    {
        GP_VM_PushBoolean(vm, false);
        return;
    }

    const auto level = parseLogLevel(levelBuffer);
    if (!level)
    {
        GP_VM_PushBoolean(vm, false);
        return;
    }

    ctx->logger().setMinLevel(*level);
    ctx->logger().info(std::string("Log level set to ") + levelBuffer);
    GP_VM_PushBoolean(vm, true);
}

extern "C" void psl_LogFilePath(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    const std::string path = bridge::FileLogSink::defaultLogPath().string();
    GP_VM_PushString(vm, path.c_str());
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

extern "C" void psl_GetLineMute(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int channel = GP_VM_PopInteger(vm);

    mixer::MixerService *const svc = mixer();
    const std::optional<bool> muted =
        svc != nullptr ? svc->getLineMute(channel) : std::nullopt;
    GP_VM_PushBoolean(vm, muted.value_or(false));
}

extern "C" void psl_SetLineLevelLinear(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const double level = GP_VM_PopDouble(vm);
    const int channel = GP_VM_PopInteger(vm);

    mixer::MixerService *const svc = mixer();
    const bool ok = svc != nullptr && svc->setLineLevelLinear(channel, level);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_GetLineLevelLinear(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int channel = GP_VM_PopInteger(vm);

    mixer::MixerService *const svc = mixer();
    const std::optional<double> level =
        svc != nullptr ? svc->getLineLevelLinear(channel) : std::nullopt;
    GP_VM_PushDouble(vm, level.value_or(0.0));
}

extern "C" void psl_SetLineSolo(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int soloedFlag = GP_VM_PopInteger(vm);
    const int channel = GP_VM_PopInteger(vm);

    mixer::MixerService *const svc = mixer();
    const bool ok = svc != nullptr && svc->setLineSolo(channel, soloedFlag != 0);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_GetLineSolo(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int channel = GP_VM_PopInteger(vm);

    mixer::MixerService *const svc = mixer();
    const std::optional<bool> soloed =
        svc != nullptr ? svc->getLineSolo(channel) : std::nullopt;
    GP_VM_PushBoolean(vm, soloed.value_or(false));
}

extern "C" void psl_SetLinePan(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const double pan = GP_VM_PopDouble(vm);
    const int channel = GP_VM_PopInteger(vm);

    mixer::MixerService *const svc = mixer();
    const bool ok = svc != nullptr && svc->setLinePan(channel, pan);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_GetLinePan(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int channel = GP_VM_PopInteger(vm);

    mixer::MixerService *const svc = mixer();
    const std::optional<double> pan =
        svc != nullptr ? svc->getLinePan(channel) : std::nullopt;
    GP_VM_PushDouble(vm, pan.value_or(50.0));
}

extern "C" void psl_SetLineColor(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    char rgbBuffer[32] = {};
    GP_VM_PopString(vm, rgbBuffer, static_cast<int>(sizeof(rgbBuffer)));
    const int channel = GP_VM_PopInteger(vm);

    mixer::MixerService *const svc = mixer();
    const bool ok = svc != nullptr && svc->setLineColor(channel, rgbBuffer);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_GetLineColor(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int channel = GP_VM_PopInteger(vm);

    mixer::MixerService *const svc = mixer();
    const std::optional<std::string> color =
        svc != nullptr ? svc->getLineColor(channel) : std::nullopt;
    GP_VM_PushString(vm, color.value_or("").c_str());
}

extern "C" void psl_GetProjectCount(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    mixer::MixerService *const svc = mixer();
    const int count = svc != nullptr ? svc->getProjectCount() : 0;
    GP_VM_PushInteger(vm, count);
}

extern "C" void psl_GetProjectName(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int index = GP_VM_PopInteger(vm);
    mixer::MixerService *const svc = mixer();
    const std::string name = svc != nullptr ? svc->getProjectName(index) : std::string{};
    GP_VM_PushString(vm, name.c_str());
}

extern "C" void psl_GetSceneCount(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    char projectBuffer[256] = {};
    GP_VM_PopString(vm, projectBuffer, static_cast<int>(sizeof(projectBuffer)));

    mixer::MixerService *const svc = mixer();
    const int count =
        svc != nullptr ? svc->getSceneCount(projectBuffer) : 0;
    GP_VM_PushInteger(vm, count);
}

extern "C" void psl_GetSceneName(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int index = GP_VM_PopInteger(vm);
    char projectBuffer[256] = {};
    GP_VM_PopString(vm, projectBuffer, static_cast<int>(sizeof(projectBuffer)));

    mixer::MixerService *const svc = mixer();
    const std::string name =
        svc != nullptr ? svc->getSceneName(projectBuffer, index) : std::string{};
    GP_VM_PushString(vm, name.c_str());
}

extern "C" void psl_RecallProjectScene(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    char sceneBuffer[256] = {};
    GP_VM_PopString(vm, sceneBuffer, static_cast<int>(sizeof(sceneBuffer)));
    char projectBuffer[256] = {};
    GP_VM_PopString(vm, projectBuffer, static_cast<int>(sizeof(projectBuffer)));

    mixer::MixerService *const svc = mixer();
    const bool ok = svc != nullptr &&
                    svc->recallProjectScene(projectBuffer, sceneBuffer);
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
    {
        "GetLineMute",
        "channel : Integer",
        "Returns Boolean",
        "Returns cached mute state for a 1-based input channel (false if unknown).",
        &psl_GetLineMute,
    },
    {
        "SetLineLevelLinear",
        "channel : Integer, level : Double",
        "Returns Boolean",
        "Set LINE channel fader level (0..100 percent).",
        &psl_SetLineLevelLinear,
    },
    {
        "GetLineLevelLinear",
        "channel : Integer",
        "Returns Double",
        "Get cached LINE fader level (0..100 percent, 0 if unknown).",
        &psl_GetLineLevelLinear,
    },
    {
        "SetLineSolo",
        "channel : Integer, soloed : Integer",
        "Returns Boolean",
        "Solo (soloed=1) or unsolo a 1-based LINE channel.",
        &psl_SetLineSolo,
    },
    {
        "GetLineSolo",
        "channel : Integer",
        "Returns Boolean",
        "Returns cached solo state (false if unknown).",
        &psl_GetLineSolo,
    },
    {
        "SetLinePan",
        "channel : Integer, pan : Double",
        "Returns Boolean",
        "Set LINE pan (0..100, 50 = center).",
        &psl_SetLinePan,
    },
    {
        "GetLinePan",
        "channel : Integer",
        "Returns Double",
        "Get cached LINE pan percent (50 if unknown).",
        &psl_GetLinePan,
    },
    {
        "SetLineColor",
        "channel : Integer, rgbHex : String",
        "Returns Boolean",
        "Set LINE channel color (RRGGBB or #RRGGBB).",
        &psl_SetLineColor,
    },
    {
        "GetLineColor",
        "channel : Integer",
        "Returns String",
        "Get cached LINE color hex (empty if unknown).",
        &psl_GetLineColor,
    },
    {
        "GetProjectCount",
        "",
        "Returns Integer",
        "List mixer projects (blocks up to ~5s on first call).",
        &psl_GetProjectCount,
    },
    {
        "GetProjectName",
        "index : Integer",
        "Returns String",
        "1-based project file name from the cached project list.",
        &psl_GetProjectName,
    },
    {
        "GetSceneCount",
        "projectFile : String",
        "Returns Integer",
        "List scenes in a project file (blocks up to ~5s on first call).",
        &psl_GetSceneCount,
    },
    {
        "GetSceneName",
        "projectFile : String, index : Integer",
        "Returns String",
        "1-based scene file name within a project.",
        &psl_GetSceneName,
    },
    {
        "RecallProjectScene",
        "projectFile : String, sceneFile : String",
        "Returns Boolean",
        "Recall a scene via FR Open (paths under presets/proj/).",
        &psl_RecallProjectScene,
    },
    {
        "SetLogLevel",
        "level : String",
        "Returns Boolean",
        "Set minimum log level: none, error, warn, info, or debug.",
        &psl_SetLogLevel,
    },
    {
        "LogFilePath",
        "",
        "Returns String",
        "Returns the path to extension.log under %APPDATA%\\PreSonusStudioLive.",
        &psl_LogFilePath,
    },
};

} // namespace

int scriptFunctionCount()
{
    return static_cast<int>(sizeof(kScriptFunctions) / sizeof(kScriptFunctions[0]));
}

ExternalAPI_GPScriptFunctionDefinition *scriptFunctions() { return kScriptFunctions; }

} // namespace presonus::studiolive::gpext::bridge
