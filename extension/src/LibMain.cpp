/// \file  LibMain.cpp

/// \brief Concrete GigPerformerAPI subclass.



#include "LibMain.h"



#include "bridge/RealGpHost.h"

#include "bridge/ScriptFunctions.h"
#include "panels/ScenePickerPanel.h"
#include "Version.h"

#include <optional>
#include <string_view>



namespace presonus::studiolive::gpext

{



// ===========================================================================

//  LibMain

// ===========================================================================



LibMain::LibMain(LibraryHandle handle)

    : gigperformer::sdk::GigPerformerAPI(handle), handle_(handle)

{

}



LibMain::~LibMain()

{

    bridge::ExtensionContext::setInstance(nullptr);

}



std::string LibMain::GetProductDescription()

{

    const std::string v = versionString();

    std::string xml;

    xml += "<Library>";

    xml += "<Product Name=\"PreSonus StudioLive\" Version=\"" + v +

           "\" BuildDate=\"" __DATE__ "\"></Product>";

    xml += "<Description>Third-party Gig Performer extension for PreSonus StudioLive III mixers (UCNet). Not affiliated with or endorsed by PreSonus. (";

    xml += v;

    xml += ", Windows).</Description>";

    xml += "<ImagePath></ImagePath>";

    xml += "</Library>";

    return xml;

}



void LibMain::Initialization()

{

    gpHost_ = std::make_unique<bridge::RealGpHost>(handle_);

    fileLog_ = std::make_unique<bridge::FileLogSink>();
    fileLog_->writeSessionBanner(versionString());

    logger_.setSink([this](bridge::LogLevel level, std::string_view message) {
        if (fileLog_ != nullptr)
        {
            fileLog_->write(level, message);
        }
        if (gpHost_ != nullptr)
        {
            gpHost_->consoleLog(std::string(message));
        }
    });



    context_ = std::make_unique<bridge::ExtensionContext>(*gpHost_, dispatcher_, logger_);

    bridge::ExtensionContext::setInstance(context_.get());

    mixer_ = std::make_unique<mixer::MixerService>(logger_);
    context_->setMixerService(mixer_.get());
    mixer_->setStateChangeCallback([this]() {
        if (context_ != nullptr)
        {
            context_->scheduleWidgetPoll();
        }
    });
    context_->setConfigStore(&config_);

    registerCallback("OnWidgetValueChanged");
    registerCallback("OnSongChanged");
    registerCallback("OnSongPartChanged");
    registerCallback("OnRackspaceActivated");
    registerCallback("OnStatusChanged");

    config_.load();

    logger_.info("PreSonus StudioLive extension initialized");

    context_->drainGpTasks();

    // Extension reload leaves the gig open without GPStatus_GigFinishedLoading.
    tryAutoConnect();

}



void LibMain::OnOpen()

{

    registerCallback("OnWidgetValueChanged");
    registerCallback("OnSongChanged");
    registerCallback("OnSongPartChanged");
    registerCallback("OnRackspaceActivated");
    registerCallback("OnStatusChanged");

    if (context_ != nullptr)

    {

        context_->drainGpTasks();

    }

    tryAutoConnect();

}



void LibMain::OnStatusChanged(const GPStatusType status)

{

    if (status == GPStatus_GigFinishedLoading)
    {
        logger_.info("Gig finished loading");
        tryAutoConnect();
        return;
    }

    if (context_ != nullptr)
    {
        context_->drainGpTasks();
    }

}



void LibMain::tryAutoConnect()

{

    if (mixer_ == nullptr)
    {
        logger_.warn("Auto-connect skipped: mixer not initialized");
        return;
    }

    if (mixer_->isConnected())
    {
        logger_.info("Auto-connect skipped: already connected");
        return;
    }

    const std::optional<std::string> preferredSerial = config_.lastSerial();
    const std::optional<std::string> preferredHost = config_.lastHost();

    if (!preferredSerial.has_value() && !preferredHost.has_value())
    {
        logger_.info("Auto-connect skipped: no saved mixer in config");
        return;
    }

    logger_.info("Auto-connect on gig load");
    bool ok = mixer_->discoverAndConnect(5000, preferredSerial, preferredHost);
    if (!ok && preferredHost.has_value() && !preferredHost->empty())
    {
        logger_.info("Discovery failed; trying last host " + *preferredHost);
        ok = mixer_->connect(*preferredHost);
    }

    if (ok)
    {
        config_.updateFromMixer(*mixer_);
        logger_.info("Auto-connect succeeded");
    }
    else
    {
        logger_.warn("Auto-connect failed");
    }

    if (context_ != nullptr)
    {
        context_->drainGpTasks();
    }

}



void LibMain::OnClose()

{

    unregisterCallback("OnWidgetValueChanged");
    unregisterCallback("OnSongChanged");
    unregisterCallback("OnSongPartChanged");
    unregisterCallback("OnRackspaceActivated");
    unregisterCallback("OnStatusChanged");

    if (mixer_ != nullptr)
    {
        mixer_->disconnect();
    }

    config_.save();

    if (context_ != nullptr)

    {

        context_->drainGpTasks();

    }

}



void LibMain::OnRackspaceActivated()

{

    if (context_ != nullptr)

    {

        context_->drainGpTasks();

    }

}



void LibMain::OnWidgetValueChanged(const std::string &widgetName, double newValue)

{

    if (context_ != nullptr && mixer_ != nullptr)

    {

        context_->widgetBindings().onWidgetValueChanged(context_->gpHost(), *mixer_,
                                                        widgetName, newValue);

        context_->drainGpTasks();

    }

}



void LibMain::OnSongChanged(int /*oldIndex*/, int newIndex)

{

    currentSongIndex_ = newIndex;

    if (context_ == nullptr || mixer_ == nullptr)
    {
        return;
    }

    context_->drainGpTasks();

    const auto binding = context_->songBindings().lookupSong(newIndex);
    if (!binding.has_value())
    {
        return;
    }

    mixer_->recallProjectScene(binding->projectFile, binding->sceneFile);
}



void LibMain::OnSongPartChanged(int /*oldIndex*/, int newIndex)

{

    if (context_ == nullptr || mixer_ == nullptr || currentSongIndex_ < 0)
    {
        return;
    }

    context_->drainGpTasks();

    const auto binding =
        context_->songBindings().lookupSongPart(currentSongIndex_, newIndex);
    if (!binding.has_value())
    {
        return;
    }

    mixer_->recallProjectScene(binding->projectFile, binding->sceneFile);
}



int LibMain::RequestGPScriptFunctionSignatureList(

    GPScript_AllowedLocations /*location*/,

    ExternalAPI_GPScriptFunctionDefinition **list)

{

    if (list != nullptr)

    {

        *list = bridge::scriptFunctions();

    }

    return bridge::scriptFunctionCount();

}



int LibMain::GetPanelCount()

{

    // Panel XML via GetPanelXML crashes GP 5.2.2 with every template we have tried
    // (hand-authored, Analog GPSCALE clones, and gp-radiobuttons-style exports).
    // Build the panel manually in GP until a safe template is confirmed.
    return 0;

}



std::string LibMain::GetPanelName(int index)

{

    if (index == 0)

    {

        return panels::kScenePickerPanelName;

    }

    return {};

}



std::string LibMain::GetPanelXML(int index)

{

    if (index != 0)

    {

        return {};

    }

    return panels::scenePickerPanelXml(getPathToMe());

}



} // namespace presonus::studiolive::gpext



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

