#pragma once

#include <string>

namespace presonus::studiolive::gpext::panels
{

inline constexpr const char *kScenePickerPanelName = "PreSonus Scene Picker";
inline constexpr const char *kScenePickerUserPanelFile = "PreSonusScenePicker.gppanel";

/// Default panel XML returned by `GetPanelXML` when no user override exists beside the DLL.
inline const char *scenePickerDefaultPanelXml()
{
    return
        "<RACKUNIT size=\"4\" style=\"0\" color=\"ff000000\" ver=\"4.099999904632568\">"
        "  <WIDGET type=\"4\" style=\"0\" x=\"0.02\" y=\"0.06\" width=\"0.60\" height=\"0.16\""
        "          caption=\"Scene\" captionVisible=\"1\" hideInPresentationView=\"0\""
        "          ignoresVariations=\"0\" widgetHandle=\"CurrentSceneLabel\" widgetValue=\"1.0\""
        "          bckgColor=\"ff3c3c3c\" outlineColor=\"ff808080\" outlineRound=\"4\""
        "          outlineThick=\"1\" fontSize=\"18.0\" textColor=\"ffffffff\">"
        "  </WIDGET>"
        "  <WIDGET type=\"3\" style=\"0\" x=\"0.64\" y=\"0.05\" width=\"0.32\" height=\"0.18\""
        "          caption=\"Select\" captionVisible=\"1\" hideInPresentationView=\"0\""
        "          ignoresVariations=\"0\" widgetHandle=\"SelectButton\" widgetValue=\"0.0\""
        "          bckgColor=\"ff3c3c3c\" outlineColor=\"ff808080\" outlineRound=\"4\""
        "          outlineThick=\"2\" btnMomToLatch=\"1\" btnPadLike=\"0\">"
        "  </WIDGET>"
        "  <WIDGET type=\"4\" style=\"0\" x=\"0.02\" y=\"0.28\" width=\"0.96\" height=\"0.10\""
        "          caption=\"Project\" captionVisible=\"1\" hideInPresentationView=\"1\""
        "          ignoresVariations=\"0\" widgetHandle=\"PickerProjectLabel\" widgetValue=\"1.0\""
        "          bckgColor=\"ff3c3c3c\" outlineColor=\"ff808080\" outlineRound=\"4\""
        "          outlineThick=\"1\" fontSize=\"14.0\" textColor=\"ffffffff\">"
        "  </WIDGET>"
        "  <WIDGET type=\"3\" style=\"0\" x=\"0.02\" y=\"0.40\" width=\"0.10\" height=\"0.12\""
        "          caption=\"&lt;\" captionVisible=\"1\" hideInPresentationView=\"1\""
        "          ignoresVariations=\"0\" widgetHandle=\"ProjPrev\" widgetValue=\"0.0\""
        "          bckgColor=\"ff3c3c3c\" outlineColor=\"ff808080\" outlineRound=\"4\""
        "          outlineThick=\"2\" btnMomToLatch=\"1\">"
        "  </WIDGET>"
        "  <WIDGET type=\"3\" style=\"0\" x=\"0.14\" y=\"0.40\" width=\"0.10\" height=\"0.12\""
        "          caption=\"&gt;\" captionVisible=\"1\" hideInPresentationView=\"1\""
        "          ignoresVariations=\"0\" widgetHandle=\"ProjNext\" widgetValue=\"0.0\""
        "          bckgColor=\"ff3c3c3c\" outlineColor=\"ff808080\" outlineRound=\"4\""
        "          outlineThick=\"2\" btnMomToLatch=\"1\">"
        "  </WIDGET>"
        "  <WIDGET type=\"4\" style=\"0\" x=\"0.78\" y=\"0.40\" width=\"0.20\" height=\"0.12\""
        "          caption=\"1/1\" captionVisible=\"1\" hideInPresentationView=\"1\""
        "          ignoresVariations=\"0\" widgetHandle=\"ProjCountLabel\" widgetValue=\"1.0\""
        "          bckgColor=\"ff2a2a2a\" outlineColor=\"ff606060\" outlineRound=\"3\""
        "          outlineThick=\"1\" fontSize=\"12.0\" textColor=\"ffcccccc\">"
        "  </WIDGET>"
        "  <WIDGET type=\"4\" style=\"0\" x=\"0.02\" y=\"0.54\" width=\"0.96\" height=\"0.10\""
        "          caption=\"Scene\" captionVisible=\"1\" hideInPresentationView=\"1\""
        "          ignoresVariations=\"0\" widgetHandle=\"PickerSceneLabel\" widgetValue=\"1.0\""
        "          bckgColor=\"ff3c3c3c\" outlineColor=\"ff808080\" outlineRound=\"4\""
        "          outlineThick=\"1\" fontSize=\"14.0\" textColor=\"ffffffff\">"
        "  </WIDGET>"
        "  <WIDGET type=\"3\" style=\"0\" x=\"0.02\" y=\"0.66\" width=\"0.10\" height=\"0.12\""
        "          caption=\"&lt;\" captionVisible=\"1\" hideInPresentationView=\"1\""
        "          ignoresVariations=\"0\" widgetHandle=\"ScenePrev\" widgetValue=\"0.0\""
        "          bckgColor=\"ff3c3c3c\" outlineColor=\"ff808080\" outlineRound=\"4\""
        "          outlineThick=\"2\" btnMomToLatch=\"1\">"
        "  </WIDGET>"
        "  <WIDGET type=\"3\" style=\"0\" x=\"0.14\" y=\"0.66\" width=\"0.10\" height=\"0.12\""
        "          caption=\"&gt;\" captionVisible=\"1\" hideInPresentationView=\"1\""
        "          ignoresVariations=\"0\" widgetHandle=\"SceneNext\" widgetValue=\"0.0\""
        "          bckgColor=\"ff3c3c3c\" outlineColor=\"ff808080\" outlineRound=\"4\""
        "          outlineThick=\"2\" btnMomToLatch=\"1\">"
        "  </WIDGET>"
        "  <WIDGET type=\"4\" style=\"0\" x=\"0.78\" y=\"0.66\" width=\"0.20\" height=\"0.12\""
        "          caption=\"1/1\" captionVisible=\"1\" hideInPresentationView=\"1\""
        "          ignoresVariations=\"0\" widgetHandle=\"SceneCountLabel\" widgetValue=\"1.0\""
        "          bckgColor=\"ff2a2a2a\" outlineColor=\"ff606060\" outlineRound=\"3\""
        "          outlineThick=\"1\" fontSize=\"12.0\" textColor=\"ffcccccc\">"
        "  </WIDGET>"
        "  <WIDGET type=\"3\" style=\"0\" x=\"0.02\" y=\"0.82\" width=\"0.46\" height=\"0.14\""
        "          caption=\"Recall\" captionVisible=\"1\" hideInPresentationView=\"1\""
        "          ignoresVariations=\"0\" widgetHandle=\"RecallButton\" widgetValue=\"0.0\""
        "          bckgColor=\"ff3c3c3c\" outlineColor=\"ff808080\" outlineRound=\"4\""
        "          outlineThick=\"2\" btnMomToLatch=\"1\">"
        "  </WIDGET>"
        "  <WIDGET type=\"3\" style=\"0\" x=\"0.52\" y=\"0.82\" width=\"0.46\" height=\"0.14\""
        "          caption=\"Done\" captionVisible=\"1\" hideInPresentationView=\"1\""
        "          ignoresVariations=\"0\" widgetHandle=\"DoneButton\" widgetValue=\"0.0\""
        "          bckgColor=\"ff3c3c3c\" outlineColor=\"ff808080\" outlineRound=\"4\""
        "          outlineThick=\"2\" btnMomToLatch=\"1\">"
        "  </WIDGET>"
        "  <WIDGET type=\"2\" style=\"0\" x=\"0.86\" y=\"0.02\" width=\"0.06\" height=\"0.08\""
        "          caption=\"P\" captionVisible=\"0\" hideInPresentationView=\"1\""
        "          ignoresVariations=\"0\" widgetHandle=\"ProjectStore\" widgetValue=\"0.0\""
        "          bckgColor=\"ff3c3c3c\" outlineColor=\"ff808080\" outlineRound=\"4\""
        "          outlineThick=\"1\">"
        "  </WIDGET>"
        "  <WIDGET type=\"2\" style=\"0\" x=\"0.93\" y=\"0.02\" width=\"0.06\" height=\"0.08\""
        "          caption=\"S\" captionVisible=\"0\" hideInPresentationView=\"1\""
        "          ignoresVariations=\"0\" widgetHandle=\"SceneStore\" widgetValue=\"0.0\""
        "          bckgColor=\"ff3c3c3c\" outlineColor=\"ff808080\" outlineRound=\"4\""
        "          outlineThick=\"1\">"
        "  </WIDGET>"
        "</RACKUNIT>";
}

/// Load `PreSonusScenePicker.gppanel` from the extension folder when present.
std::string scenePickerPanelXml(const std::string &extensionDirectory);

} // namespace presonus::studiolive::gpext::panels
