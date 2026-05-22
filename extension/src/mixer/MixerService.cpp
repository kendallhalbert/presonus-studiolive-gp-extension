#include "mixer/MixerService.h"

#include "protocol/ConnectionHandshake.h"
#include "protocol/FdParser.h"
#include "protocol/FileRequest.h"
#include "protocol/JmPacket.h"
#include "protocol/MixerConnection.h"
#include "protocol/MsParser.h"
#include "protocol/ParamKeys.h"
#include "protocol/PcEncoder.h"
#include "protocol/PvEncoder.h"
#include "protocol/PvParser.h"
#include "protocol/ValueUtil.h"
#include "transport/DiscoveryListener.h"
#include "transport/WinSockTransport.h"

#include <cctype>
#include <chrono>
#include <utility>
#include <variant>

namespace presonus::studiolive::gpext::mixer
{

namespace
{

constexpr const char *kProjectsListPath = "presets/proj";
constexpr const char *kChannelPresetsListPath = "presets/channel";

constexpr const char *kChannelPresetRecallFilterKeys[] = {
    "channelfilters/preset_eq",
    "channelfilters/preset_gate",
    "channelfilters/preset_comp",
    "channelfilters/preset_preamp",
    "channelfilters/preset_polarity",
    "channelfilters/preset_channel_type",
    "channelfilters/preset_alt_ab",
    "channelfilters/preset_aux_fxsend_pan",
    "channelfilters/preset_select_colors",
    "channelfilters/preset_48v",
    "channelfilters/preset_pan",
    "channelfilters/preset_channel_names",
    "channelfilters/preset_bus_assigns",
    "channelfilters/preset_group_assigns",
    "channelfilters/preset_mutes",
    "channelfilters/preset_faders",
};

void sendChannelPresetRecallFilters(protocol::MixerConnection &connection)
{
    for (const char *key : kChannelPresetRecallFilterKeys)
    {
        connection.sendRaw(protocol::createPvBoolPacket(key, true));
    }
}

bool isInterestingJmMessage(std::string_view json)
{
    return json.find("RestorePreset") != std::string_view::npos ||
           json.find("Preset") != std::string_view::npos ||
           json.find("Error") != std::string_view::npos ||
           json.find("error") != std::string_view::npos;
}

} // namespace

MixerService::MixerService(bridge::Logger &logger) : logger_(logger) {}

void MixerService::setStateChangeCallback(StateChangeCallback callback)
{
    stateChangeCallback_ = std::move(callback);
}

MixerService::~MixerService()
{
    if (running_.load())
    {
        enqueue([this]() {
            if (connection_)
            {
                connection_->close();
                connection_.reset();
            }
            connected_ = false;
        });
        running_ = false;
        if (ioThread_.joinable())
        {
            ioThread_.join();
        }
    }
}

void MixerService::ensureThread()
{
    if (running_.exchange(true))
    {
        return;
    }

    ioThread_ = std::thread([this]() { ioLoop(); });
}

void MixerService::stopThread()
{
    if (!running_.exchange(false))
    {
        return;
    }

    if (ioThread_.joinable())
    {
        ioThread_.join();
    }
}

void MixerService::enqueue(IoTask task)
{
    {
        std::lock_guard lock(mutex_);
        tasks_.push_back(std::move(task));
    }
}

void MixerService::ioLoop()
{
    while (running_.load())
    {
        std::vector<IoTask> batch;
        {
            std::lock_guard lock(mutex_);
            batch.swap(tasks_);
        }

        for (auto &task : batch)
        {
            if (task)
            {
                task();
            }
        }

        if (connection_)
        {
            connection_->poll();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

std::uint16_t MixerService::allocateRequestId()
{
    return nextRequestId_.fetch_add(1);
}

void MixerService::onFdListReceived(std::uint16_t requestId, std::vector<std::uint8_t> json)
{
    const std::string text(reinterpret_cast<const char *>(json.data()), json.size());
    const auto files = protocol::parseFdFileList(text);
    if (!files)
    {
        logger_.warn("FD list parse failed (requestId=" + std::to_string(requestId) +
                     ", bytes=" + std::to_string(json.size()) + ")");
        std::lock_guard lock(fdWaitMutex_);
        if (fdWait_ && fdWait_->requestId == requestId)
        {
            fdWait_->entries.clear();
            fdWait_->done.store(true);
        }
        return;
    }

    {
        std::lock_guard lock(fdWaitMutex_);
        if (fdWait_ && fdWait_->requestId == requestId)
        {
            fdWait_->entries = *files;
            fdWait_->done.store(true);
        }
    }
}

bool MixerService::fetchFileListBlocking(const std::string &path,
                                        std::vector<protocol::FdFileEntry> &out,
                                        const int maxWaitIterations)
{
    if (!isConnected())
    {
        logger_.warn("FR list skipped (not connected): " + path);
        return false;
    }

    const std::uint16_t requestId = allocateRequestId();
    auto waitState = std::make_unique<FdWaitState>();
    waitState->requestId = requestId;

    {
        std::lock_guard lock(fdWaitMutex_);
        fdWait_ = std::move(waitState);
    }

    logger_.info("FR list request: " + path + " (id=" + std::to_string(requestId) + ")");
    const auto packet = protocol::createFileListRequestPacket(requestId, path);
    enqueue([this, packet = std::move(packet)]() {
        if (connection_)
        {
            connection_->sendRaw(packet);
        }
    });

    for (int i = 0; i < maxWaitIterations; ++i)
    {
        {
            std::lock_guard lock(fdWaitMutex_);
            if (fdWait_ && fdWait_->done.load())
            {
                out = fdWait_->entries;
                fdWait_.reset();
                if (out.empty())
                {
                    logger_.warn("FR list empty: " + path);
                    return false;
                }
                logger_.info("FR list ok: " + path + " (" + std::to_string(out.size()) +
                             " entries)");
                return true;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    {
        std::lock_guard lock(fdWaitMutex_);
        fdWait_.reset();
    }
    logger_.warn("FR list timed out: " + path);
    return false;
}

bool MixerService::connect(const std::string &host, std::uint16_t port)
{
    ensureThread();

    std::atomic<bool> ok{false};
    std::atomic<bool> done{false};
    enqueue([this, host, port, &ok, &done]() {
        auto transport = std::make_unique<transport::WinSockTransport>();
        connection_ = std::make_unique<protocol::MixerConnection>(std::move(transport));
        connected_ = false;

        if (!connection_->connect(host, port))
        {
            logger_.warn("Mixer TCP connect failed: " + host);
            connection_.reset();
            ok.store(false);
            done.store(true);
            return;
        }

        stateCache_.clear();
        {
            std::lock_guard lock(catalogMutex_);
            projects_.clear();
            scenesByProject_.clear();
            channelPresets_.clear();
        }

        protocol::ConnectionHandshake handshake;
        connection_->setSessionPacketCallback(
            [&handshake, this](const protocol::SessionPacket &packet) {
                handshake.onSessionPacket(packet);
                stateCache_.apply(packet);
                if (stateChangeCallback_ &&
                    (std::holds_alternative<protocol::PvMessage>(packet.payload) ||
                     std::holds_alternative<protocol::MsMessage>(packet.payload)))
                {
                    stateChangeCallback_();
                }
            });
        connection_->setJsonMessageCallback([&handshake](std::string_view json) {
            handshake.onJmJson(json);
        });
        connection_->setFdListCallback([this](protocol::FdListResult result) {
            onFdListReceived(result.requestId, std::move(result.json));
        });
        connection_->setKeepAliveEnabled(false);

        if (!connection_->sendRaw(protocol::createSubscribePacket()))
        {
            logger_.warn("Mixer subscribe send failed: " + host);
            connection_->close();
            connection_.reset();
            ok.store(false);
            done.store(true);
            return;
        }

        const auto deadline =
            std::chrono::steady_clock::now() + std::chrono::seconds(30);
        while (!handshake.progress().complete() &&
               std::chrono::steady_clock::now() < deadline)
        {
            connection_->poll();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        if (!handshake.progress().complete())
        {
            logger_.warn("Mixer handshake timed out: " + host);
            connection_->close();
            connection_.reset();
            ok.store(false);
            done.store(true);
            return;
        }

        connection_->setJsonMessageCallback([this](std::string_view json) {
            if (isInterestingJmMessage(json))
            {
                logger_.info("JM recv: " + std::string(json));
            }
        });

        connection_->setKeepAliveEnabled(true);
        connected_ = true;
        {
            std::lock_guard lock(connectionInfoMutex_);
            connectedHost_ = host;
            if (connectedName_.empty())
            {
                if (const auto name = stateCache_.stringKey("global/mixer_name"))
                {
                    connectedName_ = *name;
                }
            }
            if (connectedSerial_.empty())
            {
                if (const auto serial = stateCache_.stringKey("global/mixer_serial"))
                {
                    connectedSerial_ = *serial;
                }
            }
        }
        logger_.info("Mixer connected to " + host);
        ok.store(true);
        done.store(true);
    });

    for (int i = 0; i < 3500 && !done.load(); ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return ok.load();
}

void MixerService::disconnect()
{
    enqueue([this]() {
        if (connection_)
        {
            connection_->close();
            connection_.reset();
        }
        connected_ = false;
        stateCache_.clear();
        {
            std::lock_guard lock(connectionInfoMutex_);
            connectedHost_.clear();
            connectedName_.clear();
            connectedSerial_.clear();
        }
        {
            std::lock_guard lock(catalogMutex_);
            projects_.clear();
            scenesByProject_.clear();
            channelPresets_.clear();
        }
        logger_.info("Mixer disconnected");
    });
}

bool MixerService::isConnected() const
{
    return connected_.load();
}

int MixerService::discover(const int timeoutMs)
{
    const auto clampedMs = timeoutMs < 0 ? 0 : timeoutMs;
    const auto devices = transport::listenForDiscovery(std::chrono::milliseconds(clampedMs), logger_);
    std::lock_guard lock(discoveryMutex_);
    discovered_ = devices;
    return static_cast<int>(discovered_.size());
}

std::string MixerService::getDiscoveredHost(const int index) const
{
    std::lock_guard lock(discoveryMutex_);
    if (index < 1 || static_cast<std::size_t>(index) > discovered_.size())
    {
        return {};
    }
    return discovered_[static_cast<std::size_t>(index) - 1].host;
}

std::string MixerService::getDiscoveredName(const int index) const
{
    std::lock_guard lock(discoveryMutex_);
    if (index < 1 || static_cast<std::size_t>(index) > discovered_.size())
    {
        return {};
    }
    return discovered_[static_cast<std::size_t>(index) - 1].name;
}

std::string MixerService::getDiscoveredSerial(const int index) const
{
    std::lock_guard lock(discoveryMutex_);
    if (index < 1 || static_cast<std::size_t>(index) > discovered_.size())
    {
        return {};
    }
    return discovered_[static_cast<std::size_t>(index) - 1].serial;
}

bool MixerService::discoverAndConnect(const int timeoutMs,
                                      const std::optional<std::string> &preferredSerial,
                                      const std::optional<std::string> &preferredHost)
{
    if (discover(timeoutMs) <= 0)
    {
        return false;
    }

    std::optional<protocol::DiscoveredMixer> chosen;
    {
        std::lock_guard lock(discoveryMutex_);
        if (preferredSerial.has_value())
        {
            for (const auto &device : discovered_)
            {
                if (device.serial == *preferredSerial)
                {
                    chosen = device;
                    break;
                }
            }
        }
        if (!chosen.has_value() && preferredHost.has_value())
        {
            for (const auto &device : discovered_)
            {
                if (device.host == *preferredHost)
                {
                    chosen = device;
                    break;
                }
            }
        }
        if (!chosen.has_value() && !discovered_.empty())
        {
            chosen = discovered_.front();
        }
    }

    if (!chosen.has_value())
    {
        return false;
    }

    std::string connectHost = chosen->host;
    if (preferredHost.has_value() && !preferredHost->empty())
    {
        const bool matchedSerial =
            preferredSerial.has_value() && chosen->serial == *preferredSerial;
        if (matchedSerial || connectHost == "127.0.0.1")
        {
            connectHost = *preferredHost;
        }
    }

    {
        std::lock_guard lock(connectionInfoMutex_);
        connectedName_ = chosen->name;
        connectedSerial_ = chosen->serial;
    }

    return connect(connectHost, chosen->tcpPort);
}

std::string MixerService::getConnectedHost() const
{
    std::lock_guard lock(connectionInfoMutex_);
    return connectedHost_;
}

std::string MixerService::getConnectedName() const
{
    std::lock_guard lock(connectionInfoMutex_);
    return connectedName_;
}

std::string MixerService::getConnectedSerial() const
{
    std::lock_guard lock(connectionInfoMutex_);
    return connectedSerial_;
}

void MixerService::sendPvFloat(const std::string &key, float value)
{
    stateCache_.setFloat(key, static_cast<double>(value));
    const auto packet = protocol::createPvPacket(key, value);
    enqueue([this, packet = std::move(packet)]() {
        if (connection_)
        {
            connection_->sendRaw(packet);
        }
    });
}

void MixerService::sendPvFloatImmediate(const std::string &key, float value)
{
    if (connection_)
    {
        connection_->sendRaw(protocol::createPvPacket(key, value));
    }
}

void MixerService::sendPvBool(const std::string &key, bool value)
{
    stateCache_.setBool(key, value);
    const auto packet = protocol::createPvBoolPacket(key, value);
    enqueue([this, packet = std::move(packet)]() {
        if (connection_)
        {
            connection_->sendRaw(packet);
        }
    });
}

bool MixerService::setLineMute(int channel, bool muted)
{
    if (channel < 1 || !isConnected())
    {
        return false;
    }

    sendPvBool(protocol::lineChannelMuteKey(channel), muted);
    return true;
}

std::optional<bool> MixerService::getLineMute(int channel) const
{
    if (channel < 1)
    {
        return std::nullopt;
    }
    return stateCache_.boolKey(protocol::lineChannelMuteKey(channel));
}

bool MixerService::setLineLevelLinear(int channel, double levelPercent)
{
    if (channel < 1 || !isConnected())
    {
        return false;
    }

    const float scalar = protocol::linearPercentToVolumeScalar(levelPercent);
    stateCache_.setFloat(protocol::lineChannelLevelKey(channel), levelPercent);
    sendPvFloat(protocol::lineChannelVolumeKey(channel), scalar);
    return true;
}

std::optional<double> MixerService::getLineLevelLinear(int channel) const
{
    if (channel < 1)
    {
        return std::nullopt;
    }

    if (const auto level = stateCache_.doubleKey(protocol::lineChannelLevelKey(channel)))
    {
        return *level;
    }
    if (const auto volume = stateCache_.doubleKey(protocol::lineChannelVolumeKey(channel)))
    {
        return protocol::volumeScalarToLinearPercent(*volume);
    }
    return std::nullopt;
}

bool MixerService::setLineLevelDb(int channel, double db)
{
    if (channel < 1 || !isConnected())
    {
        return false;
    }

    const double linear = protocol::dbToLinearPercent(db);
    return setLineLevelLinear(channel, linear);
}

std::optional<double> MixerService::getLineLevelDb(int channel) const
{
    const auto linear = getLineLevelLinear(channel);
    if (!linear.has_value())
    {
        return std::nullopt;
    }
    return protocol::linearPercentToDb(*linear);
}

bool MixerService::setLineSolo(int channel, bool soloed)
{
    if (channel < 1 || !isConnected())
    {
        return false;
    }

    sendPvBool(protocol::lineChannelSoloKey(channel), soloed);
    return true;
}

std::optional<bool> MixerService::getLineSolo(int channel) const
{
    if (channel < 1)
    {
        return std::nullopt;
    }
    return stateCache_.boolKey(protocol::lineChannelSoloKey(channel));
}

bool MixerService::setLinePan(int channel, double panPercent)
{
    if (channel < 1 || !isConnected())
    {
        return false;
    }

    sendPvFloat(protocol::lineChannelPanKey(channel), protocol::panPercentToScalar(panPercent));
    return true;
}

std::optional<double> MixerService::getLinePan(int channel) const
{
    if (channel < 1)
    {
        return std::nullopt;
    }
    if (const auto pan = stateCache_.doubleKey(protocol::lineChannelPanKey(channel)))
    {
        return protocol::panScalarToPercent(*pan);
    }
    return std::nullopt;
}

bool MixerService::setLineColor(int channel, const std::string &rgbHex)
{
    if (channel < 1 || !isConnected())
    {
        return false;
    }

    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    if (!protocol::parseRgbHex(rgbHex, r, g, b))
    {
        return false;
    }

    const auto key = protocol::lineChannelColorKey(channel);
    const auto packet = protocol::createPcPacket(key, r, g, b);

    std::string normalized;
    normalized.reserve(6);
    for (char c : rgbHex)
    {
        if (c != '#')
        {
            normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        }
    }
    stateCache_.setString(key, normalized);

    enqueue([this, packet = std::move(packet)]() {
        if (connection_)
        {
            connection_->sendRaw(packet);
        }
    });
    return true;
}

std::optional<std::string> MixerService::getLineColor(int channel) const
{
    if (channel < 1)
    {
        return std::nullopt;
    }
    return stateCache_.stringKey(protocol::lineChannelColorKey(channel));
}

bool MixerService::setChannelMute(const protocol::ChannelTarget &target, bool muted)
{
    if (!isConnected())
    {
        return false;
    }

    bool wireMuted = muted;
    if (protocol::sendMuteUsesInvertedAssign(target))
    {
        wireMuted = !muted;
    }

    sendPvBool(protocol::mutePvKey(target), wireMuted);
    return true;
}

std::optional<bool> MixerService::getChannelMute(const protocol::ChannelTarget &target) const
{
    const auto wire = stateCache_.boolKey(protocol::mutePvKey(target));
    if (!wire.has_value())
    {
        return std::nullopt;
    }

    if (protocol::sendMuteUsesInvertedAssign(target))
    {
        return !*wire;
    }
    return wire;
}

bool MixerService::setChannelLevelLinear(const protocol::ChannelTarget &target,
                                         double levelPercent,
                                         int fadeMs)
{
    if (!isConnected())
    {
        return false;
    }

    const auto key = protocol::levelPvKey(target);

    if (fadeMs <= 0)
    {
        stateCache_.setFloat(key, levelPercent);
        sendPvFloat(key, protocol::linearPercentToVolumeScalar(levelPercent));
        return true;
    }

    const double from = getChannelLevelLinear(target).value_or(levelPercent);
    if (from == levelPercent)
    {
        return true;
    }

    enqueue([this, key, from, levelPercent, fadeMs]() {
        protocol::transitionValue(
            from,
            levelPercent,
            fadeMs,
            [this, key](const double linearPercent) {
                stateCache_.setFloat(key, linearPercent);
                sendPvFloatImmediate(key,
                                     protocol::linearPercentToVolumeScalar(linearPercent));
            });
    });
    return true;
}

std::optional<double> MixerService::getChannelLevelLinear(
    const protocol::ChannelTarget &target) const
{
    const auto key = protocol::levelPvKey(target);
    if (const auto level = stateCache_.doubleKey(key))
    {
        if (*level <= 1.0)
        {
            return protocol::volumeScalarToLinearPercent(*level);
        }
        return *level;
    }
    return std::nullopt;
}

bool MixerService::setChannelLevelDb(const protocol::ChannelTarget &target,
                                       double db,
                                       int fadeMs)
{
    return setChannelLevelLinear(target, protocol::dbToLinearPercent(db), fadeMs);
}

std::optional<double> MixerService::getChannelLevelDb(
    const protocol::ChannelTarget &target) const
{
    const auto linear = getChannelLevelLinear(target);
    if (!linear.has_value())
    {
        return std::nullopt;
    }
    return protocol::linearPercentToDb(*linear);
}

bool MixerService::requestFileList(const std::string &path)
{
    if (!isConnected())
    {
        return false;
    }

    const std::uint16_t requestId = allocateRequestId();
    const auto packet = protocol::createFileListRequestPacket(requestId, path);
    enqueue([this, packet = std::move(packet)]() {
        if (connection_)
        {
            connection_->sendRaw(packet);
        }
    });
    return true;
}

std::string MixerService::sceneListPath(const std::string &projectFile)
{
    return std::string(kProjectsListPath) + "/" + projectFile;
}

std::string MixerService::sceneRecallPath(const std::string &projectFile,
                                          const std::string &sceneFile)
{
    return sceneListPath(projectFile) + "/" + sceneFile;
}

std::string MixerService::fileNameFromLoadedPath(const std::string &loadedPath)
{
    const auto pos = loadedPath.find_last_of('/');
    if (pos == std::string::npos)
    {
        return loadedPath;
    }
    return loadedPath.substr(pos + 1);
}

std::string MixerService::getCurrentProjectFile() const
{
    if (const auto loaded = stateCache_.stringKey("presets/loaded_project_name"))
    {
        return fileNameFromLoadedPath(*loaded);
    }
    return {};
}

std::string MixerService::getCurrentSceneFile() const
{
    if (const auto loaded = stateCache_.stringKey("presets/loaded_scene_name"))
    {
        return fileNameFromLoadedPath(*loaded);
    }
    return {};
}

int MixerService::getProjectCount()
{
    {
        std::lock_guard lock(catalogMutex_);
        if (!projects_.empty())
        {
            return static_cast<int>(projects_.size());
        }
    }

    std::vector<protocol::FdFileEntry> fetched;
    if (!fetchFileListBlocking(kProjectsListPath, fetched))
    {
        return 0;
    }

    {
        std::lock_guard lock(catalogMutex_);
        projects_ = std::move(fetched);
        return static_cast<int>(projects_.size());
    }
}

std::string MixerService::getProjectName(int index)
{
    std::lock_guard lock(catalogMutex_);
    if (index < 1 || static_cast<std::size_t>(index) > projects_.size())
    {
        return {};
    }
    return projects_[static_cast<std::size_t>(index) - 1].name;
}

int MixerService::getSceneCount(const std::string &projectFile)
{
    {
        std::lock_guard lock(catalogMutex_);
        const auto it = scenesByProject_.find(projectFile);
        if (it != scenesByProject_.end())
        {
            return static_cast<int>(it->second.size());
        }
    }

    std::vector<protocol::FdFileEntry> fetched;
    if (!fetchFileListBlocking(sceneListPath(projectFile), fetched))
    {
        return 0;
    }

    fetched = protocol::filterFdSceneFiles(fetched);

    {
        std::lock_guard lock(catalogMutex_);
        scenesByProject_[projectFile] = std::move(fetched);
        return static_cast<int>(scenesByProject_[projectFile].size());
    }
}

std::string MixerService::getSceneName(const std::string &projectFile, int index)
{
    std::lock_guard lock(catalogMutex_);
    const auto it = scenesByProject_.find(projectFile);
    if (it == scenesByProject_.end())
    {
        return {};
    }
    if (index < 1 || static_cast<std::size_t>(index) > it->second.size())
    {
        return {};
    }
    return it->second[static_cast<std::size_t>(index) - 1].name;
}

bool MixerService::recallProjectScene(const std::string &projectFile,
                                      const std::string &sceneFile)
{
    if (!isConnected())
    {
        return false;
    }

    const auto presetFile = sceneRecallPath(projectFile, sceneFile);
    logger_.info("Recall scene JM RestorePreset: " + presetFile);
    const auto packet = protocol::createRestorePresetPacket(presetFile);
    enqueue([this, packet = std::move(packet)]() {
        if (connection_)
        {
            connection_->sendRaw(packet);
        }
    });
    return true;
}

int MixerService::getChannelPresetCount()
{
    if (!isConnected())
    {
        logger_.warn("GetChannelPresetCount: not connected");
        return 0;
    }

    {
        std::lock_guard lock(catalogMutex_);
        if (!channelPresets_.empty())
        {
            return static_cast<int>(channelPresets_.size());
        }
    }

    std::vector<protocol::FdFileEntry> fetched;
    if (!fetchFileListBlocking(kChannelPresetsListPath, fetched))
    {
        return 0;
    }

    fetched = protocol::filterFdChannelPresetFiles(fetched);
    if (fetched.empty())
    {
        logger_.warn("GetChannelPresetCount: no channel presets after filtering");
        return 0;
    }

    {
        std::lock_guard lock(catalogMutex_);
        channelPresets_ = std::move(fetched);
        logger_.info("Channel preset count: " + std::to_string(channelPresets_.size()));
        return static_cast<int>(channelPresets_.size());
    }
}

std::string MixerService::getChannelPresetName(const int index)
{
    {
        std::lock_guard lock(catalogMutex_);
        if (!channelPresets_.empty())
        {
            if (index < 1 || static_cast<std::size_t>(index) > channelPresets_.size())
            {
                logger_.warn("GetChannelPresetName: index out of range: " +
                             std::to_string(index));
                return {};
            }
            return channelPresets_[static_cast<std::size_t>(index) - 1].name;
        }
    }

    if (getChannelPresetCount() <= 0)
    {
        return {};
    }

    std::lock_guard lock(catalogMutex_);
    if (index < 1 || static_cast<std::size_t>(index) > channelPresets_.size())
    {
        logger_.warn("GetChannelPresetName: index out of range: " + std::to_string(index));
        return {};
    }
    return channelPresets_[static_cast<std::size_t>(index) - 1].name;
}

bool MixerService::recallChannelStrip(const std::string &type,
                                      const int channel,
                                      const std::string &chanFile)
{
    if (!isConnected())
    {
        logger_.warn("RecallChannelStrip: not connected");
        return false;
    }

    if (chanFile.empty())
    {
        logger_.warn("RecallChannelStrip: empty preset file name");
        return false;
    }

    const auto target = protocol::parseChannelTarget(type, channel, "", 0);
    if (!target.has_value())
    {
        logger_.warn("RecallChannelStrip: invalid channel type/number");
        return false;
    }

    if (target->mixKind != protocol::MixKind::Main)
    {
        logger_.warn("RecallChannelStrip: main-mix channel target required");
        return false;
    }

    const auto presetFile = std::string(kChannelPresetsListPath) + "/" + chanFile;
    const auto presetTarget = protocol::channelPresetTarget(*target);
    logger_.info("Recall channel strip JM RestorePreset: " + presetFile + " -> " +
                 presetTarget);
    const auto packet = protocol::createRestorePresetPacket(presetFile, presetTarget);
    enqueue([this, packet = std::move(packet)]() {
        if (!connection_)
        {
            return;
        }

        // UC Surface recall filters default off for faders/names/etc.; enable all before load.
        sendChannelPresetRecallFilters(*connection_);
        connection_->sendRaw(packet);
    });
    return true;
}

} // namespace presonus::studiolive::gpext::mixer
