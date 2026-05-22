#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace presonus::studiolive::gpext::protocol
{

enum class MixKind
{
    Main,
    Aux,
    Fx,
};

/// Parsed GPScript channel selector (type + channel + optional send bus).
struct ChannelTarget
{
    std::string wireType;
    int channel{0};
    MixKind mixKind{MixKind::Main};
    int mixNumber{0};
};

/// Map GPScript channel type string to UCNet wire prefix (e.g. `DCA` → `filtergroup`).
std::optional<std::string> gpscriptTypeToWireType(std::string_view type);

/// Parse GPScript `type` / `mixType` strings into a wire-level target.
std::optional<ChannelTarget> parseChannelTarget(std::string_view type, int channel,
                                                std::string_view mixType, int mixNumber);

/// PV key for fader level (main `volume`, send `auxn`, or `FXA`…).
std::string levelPvKey(const ChannelTarget &target);

/// PV key for main-mix solo (`…/solo`).
std::string soloPvKey(const ChannelTarget &target);

/// PV key for main-mix pan (`…/pan`).
std::string panPvKey(const ChannelTarget &target);

/// PC key for channel color (`…/color`).
std::string colorPcKey(const ChannelTarget &target);

/// PV key for mute (`mute` or inverted `assign_auxn` / `assign_fxn`).
std::string mutePvKey(const ChannelTarget &target);

/// Send mutes use inverted assign_* semantics on the wire.
bool sendMuteUsesInvertedAssign(const ChannelTarget &target);

/// JM RestorePreset `presetTarget` for a main-mix channel (e.g. `line/ch1`).
std::string channelPresetTarget(const ChannelTarget &target);

} // namespace presonus::studiolive::gpext::protocol
