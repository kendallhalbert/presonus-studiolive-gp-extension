/// \file  LibMain.h
/// \brief Concrete GigPerformerAPI subclass for the PreSonus StudioLive
///        extension.
///
/// Phase 0: the DLL only needs to be loadable by GP and expose a single
/// GPScript function (`PreSonusStudioLive_Version()`). Subsequent phases will expand this
/// class to own the mixer Client, the Dispatcher, and the widget-binding
/// registry — but only via composition over types declared in
/// `extension/src/bridge/`. LibMain stays thin.

#pragma once

#include "bridge/ConfigStore.h"
#include "bridge/Dispatcher.h"
#include "bridge/ExtensionContext.h"
#include "bridge/Logger.h"

#include "gigperformer/sdk/GigPerformerAPI.h"
#include "gigperformer/sdk/types.h"

#include <memory>

namespace presonus::studiolive::gpext::bridge
{
class GpHost;
}

namespace presonus::studiolive::gpext
{

class LibMain final : public gigperformer::sdk::GigPerformerAPI
{
  public:
    explicit LibMain(LibraryHandle handle);
    ~LibMain() override;

    // ---- Mandatory ------------------------------------------------------
    std::string GetProductDescription() override;

    // ---- GP lifecycle ---------------------------------------------------
    void Initialization() override;
    void OnOpen() override;
    void OnClose() override;

    void OnWidgetValueChanged(const std::string &widgetName, double newValue) override;

    // ---- GPScript -------------------------------------------------------
    // Note: GPScript_AllowedLocations and ExternalAPI_GPScriptFunctionDefinition
    // are C types in the global namespace (declared inside extern "C" in
    // gigperformer/sdk/types.h), not in `gigperformer::sdk`.
    int RequestGPScriptFunctionSignatureList(
        GPScript_AllowedLocations location,
        ExternalAPI_GPScriptFunctionDefinition **list) override;

  private:
    LibraryHandle handle_{nullptr};
    std::unique_ptr<bridge::GpHost> gpHost_;
    bridge::Dispatcher dispatcher_;
    bridge::Logger logger_;
    bridge::ConfigStore config_;
    std::unique_ptr<bridge::ExtensionContext> context_;

    LibMain(const LibMain &) = delete;
    LibMain &operator=(const LibMain &) = delete;
};

} // namespace presonus::studiolive::gpext
