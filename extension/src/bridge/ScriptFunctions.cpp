#include "bridge/ScriptFunctions.h"

#include "bridge/ExtensionContext.h"
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

extern "C" void psl_Version(GPRuntimeEngine *vm)
{
    drainIfOnGpThread();
    const std::string v = presonus::studiolive::gpext::versionString();
    GP_VM_PushString(vm, v.c_str());
}

ExternalAPI_GPScriptFunctionDefinition kScriptFunctions[] = {
    {
        "Version",
        "",
        "Returns String",
        "Returns the extension version string (e.g. \"1.0.0-phase0\").",
        &psl_Version,
    },
};

} // namespace

int scriptFunctionCount()
{
    return static_cast<int>(sizeof(kScriptFunctions) / sizeof(kScriptFunctions[0]));
}

ExternalAPI_GPScriptFunctionDefinition *scriptFunctions() { return kScriptFunctions; }

} // namespace presonus::studiolive::gpext::bridge
