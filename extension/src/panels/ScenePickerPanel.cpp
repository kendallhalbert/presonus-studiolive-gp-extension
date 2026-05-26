#include "panels/ScenePickerPanel.h"

#include <fstream>
#include <sstream>

namespace presonus::studiolive::gpext::panels
{

std::string scenePickerPanelXml(const std::string &extensionDirectory)
{
    const auto path = extensionDirectory + "/" + kScenePickerUserPanelFile;
    std::ifstream input(path, std::ios::binary);
    if (!input)
    {
        return scenePickerDefaultPanelXml();
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    const std::string text = buffer.str();
    if (text.empty())
    {
        return scenePickerDefaultPanelXml();
    }
    return text;
}

} // namespace presonus::studiolive::gpext::panels
