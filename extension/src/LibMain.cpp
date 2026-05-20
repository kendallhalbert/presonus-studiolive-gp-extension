/// \file  LibMain.cpp
/// \brief Concrete GigPerformerAPI subclass + `psl_*` registration table.

#include "LibMain.h"

#include "Version.h"

#include "gigperformer/sdk/imports.h"

#include <cstring>
#include <string>

namespace presonus::studiolive::gpext
{

namespace
{

// ---------------------------------------------------------------------------
//  GPScript : psl_Version() Returns String
// ---------------------------------------------------------------------------
//
// Stack ABI (see SDK README "Accessing parameters and returning results"):
//  - Pop arguments in reverse declared order before doing anything else.
//  - Push the return value before returning.
//
// `psl_Version` takes no arguments, so we just push the version string.
extern "C" void psl_Version(GPRuntimeEngine *vm)
{
    const std::string v = versionString();
    GP_VM_PushString(vm, v.c_str());
}

// ---------------------------------------------------------------------------
//  Registration table
// ---------------------------------------------------------------------------
//
// Lives in static storage so the pointer we hand back to GP stays valid for
// the life of the DLL.  Phase 1+ will move this into a dedicated
// `bridge/ScriptFunctions.{h,cpp}` once the table grows beyond a handful of
// entries.
ExternalAPI_GPScriptFunctionDefinition kScriptFunctions[] = {
    {
        "Version",
        /* args  */ "",
        /* ret   */ "Returns String",
        /* desc  */ "Returns the extension version string (e.g. \"1.0.0-phase0\").",
        /* impl  */ &psl_Version,
    },
};

constexpr int kScriptFunctionCount =
    static_cast<int>(sizeof(kScriptFunctions) / sizeof(kScriptFunctions[0]));

} // namespace

// ===========================================================================
//  LibMain
// ===========================================================================

LibMain::LibMain(LibraryHandle handle) : gigperformer::sdk::GigPerformerAPI(handle)
{
}

LibMain::~LibMain() = default;

std::string LibMain::GetProductDescription()
{
    // GP reads Product Name= (capital N) for the Extensions list label AND for
    // the GPScript prefix (spaces/special chars stripped). Example: Name="GP HUD"
    // -> list shows "GP HUD", functions are GPHUD_DisplayText().
    // Name="PreSonus StudioLive" -> PreSonusStudioLive_Version().
    const std::string v = versionString();
    std::string xml;
    xml += "<Library>";
    xml += "<Product Name=\"PreSonus StudioLive\" Version=\"" + v +
           "\" BuildDate=\"" __DATE__ "\"></Product>";
    xml += "<Description>PreSonus StudioLive III mixer control for GPScript (";
    xml += v;
    xml += ", Windows).</Description>";
    xml += "<ImagePath></ImagePath>";
    xml += "</Library>";
    return xml;
}

void LibMain::Initialization()
{
    // Phase 1+ will spin up the Dispatcher and IO worker here.
}

void LibMain::OnOpen()
{
    // Phase 4 will trigger auto-discover + connect here.
}

void LibMain::OnClose()
{
    // Phase 1+ will close the mixer connection cleanly here.
}

int LibMain::RequestGPScriptFunctionSignatureList(
    GPScript_AllowedLocations /*location*/,
    ExternalAPI_GPScriptFunctionDefinition **list)
{
    if (list != nullptr)
    {
        *list = kScriptFunctions;
    }
    return kScriptFunctionCount;
}

} // namespace presonus::studiolive::gpext

// ===========================================================================
//  CreateGPExtension
// ---------------------------------------------------------------------------
//  Required free function — `entry.cpp` (from the SDK) calls this in both
//  GPQueryLibrary and GPRegister to instantiate the API subclass.  Declared
//  inside `namespace gigperformer::sdk` in GigPerformerAPI.h, so the
//  definition must live in the same namespace.
// ===========================================================================
namespace gigperformer
{
namespace sdk
{

GigPerformerAPI *CreateGPExtension(LibraryHandle handle)
{
    return new presonus::studiolive::gpext::LibMain(handle);
}

} // namespace sdk
} // namespace gigperformer
