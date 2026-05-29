#pragma once

#include <string>

namespace presonus::studiolive::gpext::panels
{

inline constexpr const char *kScenePickerPanelName = "PreSonus Scene Picker";
inline constexpr const char *kScenePickerUserPanelFile = "PreSonusScenePicker.gppanel";

/// Load panel XML: optional custom `.gppanel` beside the DLL, else embedded template.
/// `extensionFolder` is `getPathToMe()` (the Extensions directory).
std::string scenePickerPanelXml(const std::string &extensionFolder);

} // namespace presonus::studiolive::gpext::panels
