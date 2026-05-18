/// \file  Version.h
/// \brief Build-time version constants for the PreSonus StudioLive GP extension.
///
/// These are wired up from the top-level project via target_compile_definitions
/// (see extension/CMakeLists.txt).  Centralised here so neither LibMain nor
/// anything in src/bridge/ has to know about the macro names directly.

#pragma once

#include <string>

namespace presonus::studiolive::gpext
{

inline constexpr int kVersionMajor = PSL_VERSION_MAJOR;
inline constexpr int kVersionMinor = PSL_VERSION_MINOR;
inline constexpr int kVersionPatch = PSL_VERSION_PATCH;
inline constexpr const char *kVersionTag = PSL_VERSION_TAG;

/// Stable, human-readable version string consumed by `psl_Version()` and by
/// `GetProductDescription()`.  Format: "<MAJOR>.<MINOR>.<PATCH>-<tag>".
inline std::string versionString()
{
    std::string s = std::to_string(kVersionMajor) + "." +
                    std::to_string(kVersionMinor) + "." +
                    std::to_string(kVersionPatch);
    if (kVersionTag && kVersionTag[0] != '\0')
    {
        s += "-";
        s += kVersionTag;
    }
    return s;
}

} // namespace presonus::studiolive::gpext
