#pragma once

#include "bridge/Logger.h"
#include "protocol/ChannelKeys.h"
#include "protocol/DiscoveryParser.h"
#include "protocol/FdParser.h"
#include "state/KvCache.h"
#include "state/MeterCache.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{
class MixerConnection;
}

namespace presonus::studiolive::gpext::transport
{
class MeterListener;
}

namespace presonus::studiolive::gpext::mixer
{

/// IO-thread mixer session: TCP connect, keepalive poll, outgoing commands.
class MixerService
{
  public:
    explicit MixerService(bridge::Logger &logger);
    ~MixerService();

    MixerService(const MixerService &) = delete;
    MixerService &operator=(const MixerService &) = delete;

    bool connect(const std::string &host, std::uint16_t port = 53000);
    void disconnect();
    bool isConnected() const;

    int discover(int timeoutMs);
    std::string getDiscoveredHost(int index) const;
    std::string getDiscoveredName(int index) const;
    std::string getDiscoveredSerial(int index) const;
    bool discoverAndConnect(int timeoutMs,
                              const std::optional<std::string> &preferredSerial = std::nullopt,
                              const std::optional<std::string> &preferredHost = std::nullopt);

    std::string getConnectedHost() const;
    std::string getConnectedName() const;
    std::string getConnectedSerial() const;

    bool setLineMute(int channel, bool muted);
    std::optional<bool> getLineMute(int channel) const;

    bool setLineLevelLinear(int channel, double levelPercent);
    std::optional<double> getLineLevelLinear(int channel) const;

    bool setLineLevelDb(int channel, double db);
    std::optional<double> getLineLevelDb(int channel) const;

    bool setLineSolo(int channel, bool soloed);
    std::optional<bool> getLineSolo(int channel) const;

    bool setLinePan(int channel, double panPercent);
    std::optional<double> getLinePan(int channel) const;

    bool setLineColor(int channel, const std::string &rgbHex);
    std::optional<std::string> getLineColor(int channel) const;

    bool setChannelMute(const protocol::ChannelTarget &target, bool muted);
    std::optional<bool> getChannelMute(const protocol::ChannelTarget &target) const;

    bool setChannelLevelLinear(const protocol::ChannelTarget &target,
                               double levelPercent,
                               int fadeMs = 0);
    std::optional<double> getChannelLevelLinear(const protocol::ChannelTarget &target) const;

    bool setChannelLevelDb(const protocol::ChannelTarget &target, double db, int fadeMs = 0);
    std::optional<double> getChannelLevelDb(const protocol::ChannelTarget &target) const;

    bool setChannelSolo(const protocol::ChannelTarget &target, bool soloed);
    std::optional<bool> getChannelSolo(const protocol::ChannelTarget &target) const;

    bool setChannelPan(const protocol::ChannelTarget &target, double panPercent);
    std::optional<double> getChannelPan(const protocol::ChannelTarget &target) const;

    bool setChannelColor(const protocol::ChannelTarget &target, const std::string &rgbHex);
    std::optional<std::string> getChannelColor(const protocol::ChannelTarget &target) const;

    bool toggleChannelMute(const protocol::ChannelTarget &target);
    bool toggleChannelSolo(const protocol::ChannelTarget &target);

    int getChannelCount(const std::string &type) const;

    bool requestFileList(const std::string &path);

    int getProjectCount();
    std::string getProjectName(int index);
    int getSceneCount(const std::string &projectFile);
    std::string getSceneName(const std::string &projectFile, int index);
    bool recallProjectScene(const std::string &projectFile, const std::string &sceneFile);

    int getChannelPresetCount();
    std::string getChannelPresetName(int index);
    bool recallChannelStrip(const std::string &type, int channel, const std::string &chanFile);

    std::string getCurrentProjectFile() const;
    std::string getCurrentSceneFile() const;

    bool subscribeMeters(std::uint16_t port = 52704);
    void unsubscribeMeters();
    bool isMeterSubscribed() const;
    bool hasMeterData() const;
    std::optional<double> getMeterLevel(int groupId, int channel) const;

    /// Next `FR` / FD list request id (wraps at 16 bits).
    std::uint16_t allocateRequestId();

    using StateChangeCallback = std::function<void()>;
    void setStateChangeCallback(StateChangeCallback callback);

  private:
    using IoTask = std::function<void()>;
    struct FdWaitState
    {
        std::uint16_t requestId = 0;
        std::atomic<bool> done{false};
        std::vector<protocol::FdFileEntry> entries;
    };

    void ensureThread();
    void stopThread();
    void ioLoop();
    void enqueue(IoTask task);
    void sendPvFloat(const std::string &key, float value);
    void sendPvFloatImmediate(const std::string &key, float value);
    void sendPvBool(const std::string &key, bool value);
    void stopMeterListener();
    bool fetchFileListBlocking(const std::string &path,
                               std::vector<protocol::FdFileEntry> &out,
                               int maxWaitIterations = 1000);
    void onFdListReceived(std::uint16_t requestId, std::vector<std::uint8_t> json);
    static std::string sceneListPath(const std::string &projectFile);
    static std::string sceneRecallPath(const std::string &projectFile,
                                       const std::string &sceneFile);
    static std::string fileNameFromLoadedPath(const std::string &loadedPath);

    bridge::Logger &logger_;
    state::KvCache stateCache_;
    state::MeterCache meterCache_;
    std::unique_ptr<transport::MeterListener> meterListener_;
    std::atomic<bool> metersSubscribed_{false};
    std::uint16_t meterPort_{52704};
    std::unique_ptr<protocol::MixerConnection> connection_;
    std::thread ioThread_;
    std::mutex mutex_;
    std::vector<IoTask> tasks_;
    std::atomic<bool> running_{false};
    std::atomic<bool> connected_{false};
    std::atomic<std::uint16_t> nextRequestId_{0x1000};

    std::mutex fdWaitMutex_;
    std::unique_ptr<FdWaitState> fdWait_;

    std::mutex catalogMutex_;
    std::vector<protocol::FdFileEntry> projects_;
    std::unordered_map<std::string, std::vector<protocol::FdFileEntry>> scenesByProject_;
    std::vector<protocol::FdFileEntry> channelPresets_;

    mutable std::mutex discoveryMutex_;
    std::vector<protocol::DiscoveredMixer> discovered_;

    mutable std::mutex connectionInfoMutex_;
    std::string connectedHost_;
    std::string connectedName_;
    std::string connectedSerial_;

    StateChangeCallback stateChangeCallback_;
};

} // namespace presonus::studiolive::gpext::mixer
