#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace presonus::studiolive::gpext::bridge
{

/// Plain-text config at %APPDATA%\\PreSonusStudioLive\\config.json (Phase 4 expands).
class ConfigStore
{
  public:
    static std::filesystem::path configFilePath();

    bool load();
    bool save() const;

    const std::optional<std::string> &lastHost() const { return lastHost_; }
    void setLastHost(std::string host);

    const std::optional<std::string> &lastSerial() const { return lastSerial_; }
    void setLastSerial(std::string serial);

    const std::optional<std::string> &lastMixerName() const { return lastMixerName_; }
    void setLastMixerName(std::string name);

  private:
    std::optional<std::string> lastHost_;
    std::optional<std::string> lastSerial_;
    std::optional<std::string> lastMixerName_;
};

} // namespace presonus::studiolive::gpext::bridge
