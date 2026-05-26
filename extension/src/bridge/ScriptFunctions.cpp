#include "bridge/ScriptFunctions.h"

#include "bridge/ConfigStore.h"
#include "bridge/DisplayNameUtil.h"
#include "bridge/ExtensionContext.h"
#include "bridge/FileLogSink.h"
#include "bridge/LogLevelUtil.h"
#include "bridge/WidgetBindingRegistry.h"
#include "mixer/MixerService.h"
#include "protocol/ChannelKeys.h"
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

void finishWidgetBind(bool ok)
{
    if (ok)
    {
        drainIfOnGpThread();
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

ExtensionContext *context()
{
    return ExtensionContext::instance();
}

void persistSuccessfulConnection(ExtensionContext *ctx, mixer::MixerService *svc)
{
    if (ctx == nullptr || svc == nullptr)
    {
        return;
    }
    if (ConfigStore *config = ctx->configStore())
    {
        config->updateFromMixer(*svc);
    }
}

struct PoppedChannelSelector
{
    char type[32]{};
    int channel{0};
    char mixType[32]{};
    int mixNumber{0};
};

PoppedChannelSelector popChannelSelector(GPRuntimeEngine *vm)
{
    PoppedChannelSelector selector{};
    selector.mixNumber = GP_VM_PopInteger(vm);
    GP_VM_PopString(vm, selector.mixType, static_cast<int>(sizeof(selector.mixType)));
    selector.channel = GP_VM_PopInteger(vm);
    GP_VM_PopString(vm, selector.type, static_cast<int>(sizeof(selector.type)));
    return selector;
}

PoppedChannelSelector popMainChannelSelector(GPRuntimeEngine *vm)
{
    PoppedChannelSelector selector{};
    selector.channel = GP_VM_PopInteger(vm);
    GP_VM_PopString(vm, selector.type, static_cast<int>(sizeof(selector.type)));
    return selector;
}

std::optional<protocol::ChannelTarget> parsePoppedSelector(const PoppedChannelSelector &selector)
{
    return protocol::parseChannelTarget(selector.type, selector.channel, selector.mixType,
                                        selector.mixNumber);
}

bool bindChannelWidget(GPRuntimeEngine *vm, WidgetBindingKind kind)
{
    const int direction = GP_VM_PopInteger(vm);
    const auto selector = popChannelSelector(vm);
    char widgetBuffer[128] = {};
    GP_VM_PopString(vm, widgetBuffer, static_cast<int>(sizeof(widgetBuffer)));

    const auto target = parsePoppedSelector(selector);
    ExtensionContext *const ctx = context();
    if (ctx == nullptr || !target.has_value())
    {
        return false;
    }

    auto &bindings = ctx->widgetBindings();
    GpHost &host = ctx->gpHost();
    switch (kind)
    {
    case WidgetBindingKind::LevelLinear:
        return bindings.bindLevelLinear(host, widgetBuffer, *target,
                                        static_cast<WidgetDirection>(direction));
    case WidgetBindingKind::LevelDb:
        return bindings.bindLevelDb(host, widgetBuffer, *target,
                                     static_cast<WidgetDirection>(direction));
    case WidgetBindingKind::Mute:
        return bindings.bindMute(host, widgetBuffer, *target,
                               static_cast<WidgetDirection>(direction));
    case WidgetBindingKind::Solo:
        return bindings.bindSolo(host, widgetBuffer, *target,
                                static_cast<WidgetDirection>(direction));
    }
    return false;
}

} // namespace

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
            persistSuccessfulConnection(ctx, svc);
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

extern "C" void psl_GetConnectedHost(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    mixer::MixerService *const svc = mixer();
    const std::string host = svc != nullptr ? svc->getConnectedHost() : std::string{};
    GP_VM_PushString(vm, host.c_str());
}

extern "C" void psl_GetConnectedName(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    mixer::MixerService *const svc = mixer();
    const std::string name = svc != nullptr ? svc->getConnectedName() : std::string{};
    GP_VM_PushString(vm, name.c_str());
}

extern "C" void psl_Discover(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    const int timeoutMs = GP_VM_PopInteger(vm);
    mixer::MixerService *const svc = mixer();
    const int count = svc != nullptr ? svc->discover(timeoutMs) : 0;
    GP_VM_PushInteger(vm, count);
}

extern "C" void psl_GetDiscoveredHost(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    const int index = GP_VM_PopInteger(vm);
    mixer::MixerService *const svc = mixer();
    const std::string host = svc != nullptr ? svc->getDiscoveredHost(index) : std::string{};
    GP_VM_PushString(vm, host.c_str());
}

extern "C" void psl_GetDiscoveredName(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    const int index = GP_VM_PopInteger(vm);
    mixer::MixerService *const svc = mixer();
    const std::string name = svc != nullptr ? svc->getDiscoveredName(index) : std::string{};
    GP_VM_PushString(vm, name.c_str());
}

extern "C" void psl_GetDiscoveredSerial(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    const int index = GP_VM_PopInteger(vm);
    mixer::MixerService *const svc = mixer();
    const std::string serial = svc != nullptr ? svc->getDiscoveredSerial(index) : std::string{};
    GP_VM_PushString(vm, serial.c_str());
}

extern "C" void psl_DiscoverAndConnect(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    const int timeoutMs = GP_VM_PopInteger(vm);
    mixer::MixerService *const svc = mixer();
    if (svc == nullptr)
    {
        GP_VM_PushBoolean(vm, false);
        return;
    }

    std::optional<std::string> preferredSerial;
    std::optional<std::string> preferredHost;
    if (ExtensionContext *ctx = context())
    {
        if (ConfigStore *config = ctx->configStore())
        {
            preferredSerial = config->lastSerial();
            preferredHost = config->lastHost();
        }
    }

    const bool ok = svc->discoverAndConnect(timeoutMs, preferredSerial, preferredHost);
    if (ok)
    {
        persistSuccessfulConnection(context(), svc);
    }
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

extern "C" void psl_GetChannelPresetCount(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    mixer::MixerService *const svc = mixer();
    const int count = svc != nullptr ? svc->getChannelPresetCount() : 0;
    GP_VM_PushInteger(vm, count);
}

extern "C" void psl_GetChannelPresetName(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    const int index = GP_VM_PopInteger(vm);
    mixer::MixerService *const svc = mixer();
    const std::string name =
        svc != nullptr ? svc->getChannelPresetName(index) : std::string{};
    GP_VM_PushString(vm, name.c_str());
}

extern "C" void psl_RecallChannelStrip(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    char chanFileBuffer[256] = {};
    GP_VM_PopString(vm, chanFileBuffer, static_cast<int>(sizeof(chanFileBuffer)));
    const int channel = GP_VM_PopInteger(vm);
    char typeBuffer[32] = {};
    GP_VM_PopString(vm, typeBuffer, static_cast<int>(sizeof(typeBuffer)));

    mixer::MixerService *const svc = mixer();
    const bool ok = svc != nullptr &&
                    svc->recallChannelStrip(typeBuffer, channel, chanFileBuffer);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_GetCurrentProject(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    mixer::MixerService *const svc = mixer();
    const std::string name = svc != nullptr ? svc->getCurrentProjectFile() : std::string{};
    GP_VM_PushString(vm, name.c_str());
}

extern "C" void psl_GetCurrentScene(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    mixer::MixerService *const svc = mixer();
    const std::string name = svc != nullptr ? svc->getCurrentSceneFile() : std::string{};
    GP_VM_PushString(vm, name.c_str());
}

extern "C" void psl_GetSceneDisplayName(GPRuntimeEngine *vm)
{
    char sceneFileBuffer[256] = {};
    GP_VM_PopString(vm, sceneFileBuffer, static_cast<int>(sizeof(sceneFileBuffer)));
    const std::string display = presetFileDisplayName(sceneFileBuffer);
    GP_VM_PushString(vm, display.c_str());
}

extern "C" void psl_GetProjectDisplayName(GPRuntimeEngine *vm)
{
    char projectFileBuffer[256] = {};
    GP_VM_PopString(vm, projectFileBuffer, static_cast<int>(sizeof(projectFileBuffer)));
    const std::string display = presetFileDisplayName(projectFileBuffer);
    GP_VM_PushString(vm, display.c_str());
}

extern "C" void psl_SetMute(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int mutedFlag = GP_VM_PopInteger(vm);
    const int mixNumber = GP_VM_PopInteger(vm);
    char mixTypeBuffer[32] = {};
    GP_VM_PopString(vm, mixTypeBuffer, static_cast<int>(sizeof(mixTypeBuffer)));
    const int channel = GP_VM_PopInteger(vm);
    char typeBuffer[32] = {};
    GP_VM_PopString(vm, typeBuffer, static_cast<int>(sizeof(typeBuffer)));

    mixer::MixerService *const svc = mixer();
    const auto target =
        protocol::parseChannelTarget(typeBuffer, channel, mixTypeBuffer, mixNumber);
    const bool ok = svc != nullptr && target.has_value() &&
                    svc->setChannelMute(*target, mutedFlag != 0);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_GetMute(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int mixNumber = GP_VM_PopInteger(vm);
    char mixTypeBuffer[32] = {};
    GP_VM_PopString(vm, mixTypeBuffer, static_cast<int>(sizeof(mixTypeBuffer)));
    const int channel = GP_VM_PopInteger(vm);
    char typeBuffer[32] = {};
    GP_VM_PopString(vm, typeBuffer, static_cast<int>(sizeof(typeBuffer)));

    mixer::MixerService *const svc = mixer();
    const auto target =
        protocol::parseChannelTarget(typeBuffer, channel, mixTypeBuffer, mixNumber);
    if (svc == nullptr || !target.has_value())
    {
        GP_VM_PushBoolean(vm, false);
        return;
    }

    const std::optional<bool> muted = svc->getChannelMute(*target);
    GP_VM_PushBoolean(vm, muted.value_or(false));
}

extern "C" void psl_SetLevelLinear(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int fadeMs = GP_VM_PopInteger(vm);
    const double level = GP_VM_PopDouble(vm);
    const int mixNumber = GP_VM_PopInteger(vm);
    char mixTypeBuffer[32] = {};
    GP_VM_PopString(vm, mixTypeBuffer, static_cast<int>(sizeof(mixTypeBuffer)));
    const int channel = GP_VM_PopInteger(vm);
    char typeBuffer[32] = {};
    GP_VM_PopString(vm, typeBuffer, static_cast<int>(sizeof(typeBuffer)));

    mixer::MixerService *const svc = mixer();
    const auto target =
        protocol::parseChannelTarget(typeBuffer, channel, mixTypeBuffer, mixNumber);
    const bool ok = svc != nullptr && target.has_value() &&
                    svc->setChannelLevelLinear(*target, level, fadeMs);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_GetLevelLinear(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int mixNumber = GP_VM_PopInteger(vm);
    char mixTypeBuffer[32] = {};
    GP_VM_PopString(vm, mixTypeBuffer, static_cast<int>(sizeof(mixTypeBuffer)));
    const int channel = GP_VM_PopInteger(vm);
    char typeBuffer[32] = {};
    GP_VM_PopString(vm, typeBuffer, static_cast<int>(sizeof(typeBuffer)));

    mixer::MixerService *const svc = mixer();
    const auto target =
        protocol::parseChannelTarget(typeBuffer, channel, mixTypeBuffer, mixNumber);
    if (svc == nullptr || !target.has_value())
    {
        GP_VM_PushDouble(vm, 0.0);
        return;
    }

    const std::optional<double> level = svc->getChannelLevelLinear(*target);
    GP_VM_PushDouble(vm, level.value_or(0.0));
}

extern "C" void psl_SetLevelDb(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int fadeMs = GP_VM_PopInteger(vm);
    const double db = GP_VM_PopDouble(vm);
    const int mixNumber = GP_VM_PopInteger(vm);
    char mixTypeBuffer[32] = {};
    GP_VM_PopString(vm, mixTypeBuffer, static_cast<int>(sizeof(mixTypeBuffer)));
    const int channel = GP_VM_PopInteger(vm);
    char typeBuffer[32] = {};
    GP_VM_PopString(vm, typeBuffer, static_cast<int>(sizeof(typeBuffer)));

    mixer::MixerService *const svc = mixer();
    const auto target =
        protocol::parseChannelTarget(typeBuffer, channel, mixTypeBuffer, mixNumber);
    const bool ok = svc != nullptr && target.has_value() &&
                    svc->setChannelLevelDb(*target, db, fadeMs);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_GetLevelDb(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int mixNumber = GP_VM_PopInteger(vm);
    char mixTypeBuffer[32] = {};
    GP_VM_PopString(vm, mixTypeBuffer, static_cast<int>(sizeof(mixTypeBuffer)));
    const int channel = GP_VM_PopInteger(vm);
    char typeBuffer[32] = {};
    GP_VM_PopString(vm, typeBuffer, static_cast<int>(sizeof(typeBuffer)));

    mixer::MixerService *const svc = mixer();
    const auto target =
        protocol::parseChannelTarget(typeBuffer, channel, mixTypeBuffer, mixNumber);
    if (svc == nullptr || !target.has_value())
    {
        GP_VM_PushDouble(vm, -84.0);
        return;
    }

    const std::optional<double> db = svc->getChannelLevelDb(*target);
    GP_VM_PushDouble(vm, db.value_or(-84.0));
}

extern "C" void psl_GetChannelCount(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    char typeBuffer[32] = {};
    GP_VM_PopString(vm, typeBuffer, static_cast<int>(sizeof(typeBuffer)));

    mixer::MixerService *const svc = mixer();
    const int count = svc != nullptr ? svc->getChannelCount(typeBuffer) : 0;
    GP_VM_PushInteger(vm, count);
}

extern "C" void psl_ToggleMute(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const auto selector = popChannelSelector(vm);
    mixer::MixerService *const svc = mixer();
    const auto target = parsePoppedSelector(selector);
    const bool ok =
        svc != nullptr && target.has_value() && svc->toggleChannelMute(*target);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_SetSolo(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int soloedFlag = GP_VM_PopInteger(vm);
    const auto selector = popMainChannelSelector(vm);
    mixer::MixerService *const svc = mixer();
    const auto target = parsePoppedSelector(selector);
    const bool ok = svc != nullptr && target.has_value() &&
                    svc->setChannelSolo(*target, soloedFlag != 0);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_GetSolo(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const auto selector = popMainChannelSelector(vm);
    mixer::MixerService *const svc = mixer();
    const auto target = parsePoppedSelector(selector);
    if (svc == nullptr || !target.has_value())
    {
        GP_VM_PushBoolean(vm, false);
        return;
    }

    const std::optional<bool> soloed = svc->getChannelSolo(*target);
    GP_VM_PushBoolean(vm, soloed.value_or(false));
}

extern "C" void psl_ToggleSolo(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const auto selector = popMainChannelSelector(vm);
    mixer::MixerService *const svc = mixer();
    const auto target = parsePoppedSelector(selector);
    const bool ok =
        svc != nullptr && target.has_value() && svc->toggleChannelSolo(*target);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_SetPan(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const double pan = GP_VM_PopDouble(vm);
    const auto selector = popMainChannelSelector(vm);
    mixer::MixerService *const svc = mixer();
    const auto target = parsePoppedSelector(selector);
    const bool ok = svc != nullptr && target.has_value() &&
                    svc->setChannelPan(*target, pan);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_GetPan(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const auto selector = popMainChannelSelector(vm);
    mixer::MixerService *const svc = mixer();
    const auto target = parsePoppedSelector(selector);
    if (svc == nullptr || !target.has_value())
    {
        GP_VM_PushDouble(vm, 50.0);
        return;
    }

    const std::optional<double> pan = svc->getChannelPan(*target);
    GP_VM_PushDouble(vm, pan.value_or(50.0));
}

extern "C" void psl_SetColor(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    char rgbBuffer[32] = {};
    GP_VM_PopString(vm, rgbBuffer, static_cast<int>(sizeof(rgbBuffer)));
    const auto selector = popMainChannelSelector(vm);
    mixer::MixerService *const svc = mixer();
    const auto target = parsePoppedSelector(selector);
    const bool ok = svc != nullptr && target.has_value() &&
                    svc->setChannelColor(*target, rgbBuffer);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_GetColor(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const auto selector = popMainChannelSelector(vm);
    mixer::MixerService *const svc = mixer();
    const auto target = parsePoppedSelector(selector);
    const std::optional<std::string> color =
        svc != nullptr && target.has_value() ? svc->getChannelColor(*target)
                                             : std::nullopt;
    GP_VM_PushString(vm, color.value_or("").c_str());
}

extern "C" void psl_BindLevelWidgetLinear(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    const bool ok = bindChannelWidget(vm, WidgetBindingKind::LevelLinear);
    finishWidgetBind(ok);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_BindLevelWidgetDb(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    const bool ok = bindChannelWidget(vm, WidgetBindingKind::LevelDb);
    finishWidgetBind(ok);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_BindMuteWidget(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    const bool ok = bindChannelWidget(vm, WidgetBindingKind::Mute);
    finishWidgetBind(ok);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_BindSoloWidget(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int direction = GP_VM_PopInteger(vm);
    const auto selector = popMainChannelSelector(vm);
    char widgetBuffer[128] = {};
    GP_VM_PopString(vm, widgetBuffer, static_cast<int>(sizeof(widgetBuffer)));

    const auto target = parsePoppedSelector(selector);
    ExtensionContext *const ctx = context();
    const bool ok = ctx != nullptr && target.has_value() &&
                    ctx->widgetBindings().bindSolo(
                        ctx->gpHost(), widgetBuffer, *target,
                        static_cast<WidgetDirection>(direction));
    finishWidgetBind(ok);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_BindLineLevelWidgetLinear(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int direction = GP_VM_PopInteger(vm);
    const int channel = GP_VM_PopInteger(vm);
    char widgetBuffer[128] = {};
    GP_VM_PopString(vm, widgetBuffer, static_cast<int>(sizeof(widgetBuffer)));

    ExtensionContext *const ctx = context();
    const bool ok =
        ctx != nullptr &&
        ctx->widgetBindings().bindLineLevelLinear(
            ctx->gpHost(), widgetBuffer, channel,
            static_cast<WidgetDirection>(direction));
    if (!ok && ctx != nullptr)
    {
        ctx->logger().warn(std::string("BindLineLevelWidgetLinear failed for widget '") +
                           widgetBuffer + "'");
    }
    finishWidgetBind(ok);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_BindLineLevelWidgetDb(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int direction = GP_VM_PopInteger(vm);
    const int channel = GP_VM_PopInteger(vm);
    char widgetBuffer[128] = {};
    GP_VM_PopString(vm, widgetBuffer, static_cast<int>(sizeof(widgetBuffer)));

    ExtensionContext *const ctx = context();
    const bool ok =
        ctx != nullptr &&
        ctx->widgetBindings().bindLineLevelDb(
            ctx->gpHost(), widgetBuffer, channel,
            static_cast<WidgetDirection>(direction));
    if (!ok && ctx != nullptr)
    {
        ctx->logger().warn(std::string("BindLineLevelWidgetDb failed for widget '") +
                           widgetBuffer + "'");
    }
    finishWidgetBind(ok);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_BindLineMuteWidget(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int direction = GP_VM_PopInteger(vm);
    const int channel = GP_VM_PopInteger(vm);
    char widgetBuffer[128] = {};
    GP_VM_PopString(vm, widgetBuffer, static_cast<int>(sizeof(widgetBuffer)));

    ExtensionContext *const ctx = context();
    const bool ok =
        ctx != nullptr &&
        ctx->widgetBindings().bindLineMute(
            ctx->gpHost(), widgetBuffer, channel,
            static_cast<WidgetDirection>(direction));
    if (!ok && ctx != nullptr)
    {
        ctx->logger().warn(std::string("BindLineMuteWidget failed for widget '") +
                           widgetBuffer +
                           "' (use a Switch/toggle widget, not a momentary Button)");
    }
    finishWidgetBind(ok);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_BindLineSoloWidget(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int direction = GP_VM_PopInteger(vm);
    const int channel = GP_VM_PopInteger(vm);
    char widgetBuffer[128] = {};
    GP_VM_PopString(vm, widgetBuffer, static_cast<int>(sizeof(widgetBuffer)));

    ExtensionContext *const ctx = context();
    const bool ok =
        ctx != nullptr &&
        ctx->widgetBindings().bindLineSolo(
            ctx->gpHost(), widgetBuffer, channel,
            static_cast<WidgetDirection>(direction));
    if (!ok && ctx != nullptr)
    {
        ctx->logger().warn(std::string("BindLineSoloWidget failed for widget '") +
                           widgetBuffer + "'");
    }
    finishWidgetBind(ok);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_UnbindWidget(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    char widgetBuffer[128] = {};
    GP_VM_PopString(vm, widgetBuffer, static_cast<int>(sizeof(widgetBuffer)));

    ExtensionContext *const ctx = context();
    const bool ok =
        ctx != nullptr && ctx->widgetBindings().unbind(ctx->gpHost(), widgetBuffer);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_UnbindAll(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    (void)vm;

    if (ExtensionContext *const ctx = context())
    {
        ctx->widgetBindings().unbindAll(ctx->gpHost());
    }
}

extern "C" void psl_PollWidgetBindings(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    (void)vm;
}

extern "C" void psl_SubscribeMeters(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    (void)vm;

    mixer::MixerService *const svc = mixer();
    const bool ok = svc != nullptr && svc->subscribeMeters();
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_UnsubscribeMeters(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    (void)vm;

    if (mixer::MixerService *const svc = mixer())
    {
        svc->unsubscribeMeters();
    }
}

extern "C" void psl_GetMeterLevel(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int channel = GP_VM_PopInteger(vm);
    const int groupId = GP_VM_PopInteger(vm);

    mixer::MixerService *const svc = mixer();
    const std::optional<double> level =
        svc != nullptr ? svc->getMeterLevel(groupId, channel) : std::nullopt;
    GP_VM_PushDouble(vm, level.value_or(0.0));
}

extern "C" void psl_HasMeterData(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    (void)vm;

    mixer::MixerService *const svc = mixer();
    GP_VM_PushBoolean(vm, svc != nullptr && svc->hasMeterData());
}

extern "C" void psl_BindMeterWidget(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int direction = GP_VM_PopInteger(vm);
    const int channel = GP_VM_PopInteger(vm);
    const int groupId = GP_VM_PopInteger(vm);
    char widgetBuffer[128] = {};
    GP_VM_PopString(vm, widgetBuffer, static_cast<int>(sizeof(widgetBuffer)));

    ExtensionContext *const ctx = context();
    const bool ok = ctx != nullptr &&
                    ctx->widgetBindings().bindMeter(
                        ctx->gpHost(), widgetBuffer, groupId, channel,
                        static_cast<WidgetDirection>(direction));
    if (!ok && ctx != nullptr)
    {
        ctx->logger().warn(std::string("BindMeterWidget failed for widget '") + widgetBuffer +
                           "' groupId=" + std::to_string(groupId) +
                           " channel=" + std::to_string(channel) +
                           " direction=" + std::to_string(direction) +
                           " (need direction 0 or 2, widget on panel, channel 1..32)");
    }
    finishWidgetBind(ok);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_BindSongToScene(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    char sceneBuffer[256] = {};
    GP_VM_PopString(vm, sceneBuffer, static_cast<int>(sizeof(sceneBuffer)));
    char projectBuffer[256] = {};
    GP_VM_PopString(vm, projectBuffer, static_cast<int>(sizeof(projectBuffer)));
    const int songIndex = GP_VM_PopInteger(vm);

    ExtensionContext *const ctx = context();
    const bool ok = ctx != nullptr &&
                    ctx->songBindings().bindSong(songIndex, projectBuffer, sceneBuffer);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_BindSongPartToScene(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    char sceneBuffer[256] = {};
    GP_VM_PopString(vm, sceneBuffer, static_cast<int>(sizeof(sceneBuffer)));
    char projectBuffer[256] = {};
    GP_VM_PopString(vm, projectBuffer, static_cast<int>(sizeof(projectBuffer)));
    const int partIndex = GP_VM_PopInteger(vm);
    const int songIndex = GP_VM_PopInteger(vm);

    ExtensionContext *const ctx = context();
    const bool ok =
        ctx != nullptr &&
        ctx->songBindings().bindSongPart(songIndex, partIndex, projectBuffer, sceneBuffer);
    GP_VM_PushBoolean(vm, ok);
}

extern "C" void psl_UnbindSong(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();

    const int songIndex = GP_VM_PopInteger(vm);
    ExtensionContext *const ctx = context();
    const bool ok = ctx != nullptr && ctx->songBindings().unbindSong(songIndex);
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
        "GetConnectedHost",
        "",
        "Returns String",
        "IP/hostname of the active mixer session (empty if disconnected).",
        &psl_GetConnectedHost,
    },
    {
        "GetConnectedName",
        "",
        "Returns String",
        "Mixer model/name from discovery or handshake (empty if unknown).",
        &psl_GetConnectedName,
    },
    {
        "Discover",
        "timeoutMs : Integer",
        "Returns Integer",
        "Listen for UCNet UDP broadcasts on port 47809 for timeoutMs. Returns device count.",
        &psl_Discover,
    },
    {
        "GetDiscoveredHost",
        "index : Integer",
        "Returns String",
        "1-based IP/hostname from the last Discover() call.",
        &psl_GetDiscoveredHost,
    },
    {
        "GetDiscoveredName",
        "index : Integer",
        "Returns String",
        "1-based mixer model name from the last Discover() call.",
        &psl_GetDiscoveredName,
    },
    {
        "GetDiscoveredSerial",
        "index : Integer",
        "Returns String",
        "1-based mixer serial from the last Discover() call.",
        &psl_GetDiscoveredSerial,
    },
    {
        "DiscoverAndConnect",
        "timeoutMs : Integer",
        "Returns Boolean",
        "Discover mixers, prefer last serial/host from config, then TCP connect (port 53000).",
        &psl_DiscoverAndConnect,
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
        "List recallable scenes (.scn) in a project (blocks up to ~5s on first call).",
        &psl_GetSceneCount,
    },
    {
        "GetSceneName",
        "projectFile : String, index : Integer",
        "Returns String",
        "1-based scene file name (.scn) within a project.",
        &psl_GetSceneName,
    },
    {
        "RecallProjectScene",
        "projectFile : String, sceneFile : String",
        "Returns Boolean",
        "Recall a scene via JM RestorePreset (presetFile under presets/proj/).",
        &psl_RecallProjectScene,
    },
    {
        "GetChannelPresetCount",
        "",
        "Returns Integer",
        "List channel presets stored on the mixer (blocks up to ~5s on first call).",
        &psl_GetChannelPresetCount,
    },
    {
        "GetChannelPresetName",
        "index : Integer",
        "Returns String",
        "1-based channel preset file name from the cached list.",
        &psl_GetChannelPresetName,
    },
    {
        "RecallChannelStrip",
        "type : String, channel : Integer, chanFile : String",
        "Returns Boolean",
        "Recall a channel preset onto a main-mix channel via JM RestorePreset.",
        &psl_RecallChannelStrip,
    },
    {
        "GetCurrentProject",
        "",
        "Returns String",
        "Loaded project file name from mixer state (empty if unknown).",
        &psl_GetCurrentProject,
    },
    {
        "GetCurrentScene",
        "",
        "Returns String",
        "Loaded scene file name from mixer state (empty if unknown).",
        &psl_GetCurrentScene,
    },
    {
        "GetSceneDisplayName",
        "sceneFile : String",
        "Returns String",
        "Friendly scene title from a .scn filename (e.g. 01.Live Performance.scn).",
        &psl_GetSceneDisplayName,
    },
    {
        "GetProjectDisplayName",
        "projectFile : String",
        "Returns String",
        "Friendly project title from a .proj filename (e.g. 01.West End Girls.proj).",
        &psl_GetProjectDisplayName,
    },
    {
        "SetMute",
        "type : String, channel : Integer, mixType : String, mixNumber : Integer, muted : Integer",
        "Returns Boolean",
        "Mute/unmute a channel or send (mixType AUX/FX; empty mixType = main).",
        &psl_SetMute,
    },
    {
        "GetMute",
        "type : String, channel : Integer, mixType : String, mixNumber : Integer",
        "Returns Boolean",
        "Read mute/send state (AUX/FX assign uses inverted semantics).",
        &psl_GetMute,
    },
    {
        "SetLevelLinear",
        "type : String, channel : Integer, mixType : String, mixNumber : Integer, level : Double, fadeMs : Integer",
        "Returns Boolean",
        "Set fader level 0..100 (main volume or AUX/FX send level). fadeMs 0 = instant.",
        &psl_SetLevelLinear,
    },
    {
        "GetLevelLinear",
        "type : String, channel : Integer, mixType : String, mixNumber : Integer",
        "Returns Double",
        "Read fader level 0..100.",
        &psl_GetLevelLinear,
    },
    {
        "SetLevelDb",
        "type : String, channel : Integer, mixType : String, mixNumber : Integer, db : Double, fadeMs : Integer",
        "Returns Boolean",
        "Set fader level in dB -84..+10. fadeMs 0 = instant.",
        &psl_SetLevelDb,
    },
    {
        "GetLevelDb",
        "type : String, channel : Integer, mixType : String, mixNumber : Integer",
        "Returns Double",
        "Read fader level in dB.",
        &psl_GetLevelDb,
    },
    {
        "GetChannelCount",
        "type : String",
        "Returns Integer",
        "Return channel count for type (LINE, DCA, SUB, etc.) from mixer state cache.",
        &psl_GetChannelCount,
    },
    {
        "ToggleMute",
        "type : String, channel : Integer, mixType : String, mixNumber : Integer",
        "Returns Boolean",
        "Toggle mute for any channel type / send.",
        &psl_ToggleMute,
    },
    {
        "SetSolo",
        "type : String, channel : Integer, soloed : Integer",
        "Returns Boolean",
        "Set main-mix solo (soloed=1) for LINE, RETURN, DCA, SUB, etc.",
        &psl_SetSolo,
    },
    {
        "GetSolo",
        "type : String, channel : Integer",
        "Returns Boolean",
        "Read main-mix solo state.",
        &psl_GetSolo,
    },
    {
        "ToggleSolo",
        "type : String, channel : Integer",
        "Returns Boolean",
        "Toggle main-mix solo.",
        &psl_ToggleSolo,
    },
    {
        "SetPan",
        "type : String, channel : Integer, pan : Double",
        "Returns Boolean",
        "Set main-mix pan 0..100 for any channel type.",
        &psl_SetPan,
    },
    {
        "GetPan",
        "type : String, channel : Integer",
        "Returns Double",
        "Read main-mix pan 0..100.",
        &psl_GetPan,
    },
    {
        "SetColor",
        "type : String, channel : Integer, rgbHex : String",
        "Returns Boolean",
        "Set channel color (hex RGB) for any main-mix channel type.",
        &psl_SetColor,
    },
    {
        "GetColor",
        "type : String, channel : Integer",
        "Returns String",
        "Read channel color hex.",
        &psl_GetColor,
    },
    {
        "BindLevelWidgetLinear",
        "widgetName : String, type : String, channel : Integer, mixType : String, mixNumber : Integer, direction : Integer",
        "Returns Boolean",
        "Bind widget 0..1 to fader 0..100% for any channel type.",
        &psl_BindLevelWidgetLinear,
    },
    {
        "BindLevelWidgetDb",
        "widgetName : String, type : String, channel : Integer, mixType : String, mixNumber : Integer, direction : Integer",
        "Returns Boolean",
        "Bind widget 0..1 to fader dB for any channel type.",
        &psl_BindLevelWidgetDb,
    },
    {
        "BindMuteWidget",
        "widgetName : String, type : String, channel : Integer, mixType : String, mixNumber : Integer, direction : Integer",
        "Returns Boolean",
        "Bind widget to mute for any channel type / send.",
        &psl_BindMuteWidget,
    },
    {
        "BindSoloWidget",
        "widgetName : String, type : String, channel : Integer, direction : Integer",
        "Returns Boolean",
        "Bind widget to main-mix solo for any channel type.",
        &psl_BindSoloWidget,
    },
    {
        "BindLineLevelWidgetLinear",
        "widgetName : String, channel : Integer, direction : Integer",
        "Returns Boolean",
        "Bind GP widget 0..1 to LINE fader 0..100% (direction 0/1/2).",
        &psl_BindLineLevelWidgetLinear,
    },
    {
        "BindLineLevelWidgetDb",
        "widgetName : String, channel : Integer, direction : Integer",
        "Returns Boolean",
        "Bind GP widget 0..1 to LINE fader dB -84..+10.",
        &psl_BindLineLevelWidgetDb,
    },
    {
        "BindLineMuteWidget",
        "widgetName : String, channel : Integer, direction : Integer",
        "Returns Boolean",
        "Bind GP widget to LINE mute (>=0.5 = muted).",
        &psl_BindLineMuteWidget,
    },
    {
        "BindLineSoloWidget",
        "widgetName : String, channel : Integer, direction : Integer",
        "Returns Boolean",
        "Bind GP widget to LINE solo.",
        &psl_BindLineSoloWidget,
    },
    {
        "UnbindWidget",
        "widgetName : String",
        "Returns Boolean",
        "Remove a widget binding.",
        &psl_UnbindWidget,
    },
    {
        "UnbindAll",
        "",
        "",
        "Remove all widget bindings.",
        &psl_UnbindAll,
    },
    {
        "PollWidgetBindings",
        "",
        "",
        "Drain queued mixer-to-widget updates on the GP thread. Call from On TimerTick when direction includes mixer-to-widget.",
        &psl_PollWidgetBindings,
    },
    {
        "SubscribeMeters",
        "",
        "Returns Boolean",
        "Open UDP port 52704 and notify the mixer (UM Hello). Requires Connect first.",
        &psl_SubscribeMeters,
    },
    {
        "UnsubscribeMeters",
        "",
        "",
        "Stop the UDP meter listener.",
        &psl_UnsubscribeMeters,
    },
    {
        "GetMeterLevel",
        "groupId : Integer, channel : Integer",
        "Returns Double",
        "Cached meter level 0..100 for groupId 0..4 and channel 1..32 (non-blocking).",
        &psl_GetMeterLevel,
    },
    {
        "HasMeterData",
        "",
        "Returns Boolean",
        "True after at least one UCNet levl UDP frame has been received.",
        &psl_HasMeterData,
    },
    {
        "BindMeterWidget",
        "widgetName : String, groupId : Integer, channel : Integer, direction : Integer",
        "Returns Boolean",
        "Mirror UCNet meter level to a GP widget (direction 0 or 2 only). Call PollWidgetBindings from On TimerTick.",
        &psl_BindMeterWidget,
    },
    {
        "BindSongToScene",
        "songIndex : Integer, projectFile : String, sceneFile : String",
        "Returns Boolean",
        "Recall scene when GP song changes (OnSongChanged).",
        &psl_BindSongToScene,
    },
    {
        "BindSongPartToScene",
        "songIndex : Integer, partIndex : Integer, projectFile : String, sceneFile : String",
        "Returns Boolean",
        "Recall scene when GP song part changes.",
        &psl_BindSongPartToScene,
    },
    {
        "UnbindSong",
        "songIndex : Integer",
        "Returns Boolean",
        "Remove song/scene binding for a song index.",
        &psl_UnbindSong,
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

int scriptFunctionCount()
{
    return static_cast<int>(sizeof(kScriptFunctions) / sizeof(kScriptFunctions[0]));
}

ExternalAPI_GPScriptFunctionDefinition *scriptFunctions() { return kScriptFunctions; }

} // namespace presonus::studiolive::gpext::bridge
