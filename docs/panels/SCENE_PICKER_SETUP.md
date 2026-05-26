# Scene picker panel setup

Collapsed rackspace UI: **scene label + Select button**. Select opens a project/scene
browser; **Recall** loads the desk scene; **Done** closes without recalling. Each rack
**variation** stores its own project/scene selection and **auto-recalls** when you switch
variations.

## Requirements

- Release build of `PreSonusStudioLive.dll` (GP 5 + SDK v62)
- Extension **auto-connect** enabled (saved mixer in `%APPDATA%\PreSonusStudioLive\config.json`)
- StudioLive on the LAN (TCP 53000)

## Install

1. Install or rebuild the extension DLL into Gig Performer’s **Extensions** folder.
2. **Options → Reload Third Party Libraries** and enable **PreSonus StudioLive**.

## Add the panel

1. Open a rackspace in **Panels** view → **Edit** mode.
2. **New panel → PreSonus Scene Picker** (from the extension template).
3. Open **Window → Show Rackspace Script Editor**.
4. Paste the contents of [`scene-picker-collapsed.gpscript`](scene-picker-collapsed.gpscript).
5. Save the gig.

### Custom panel layout

Export your edited panel as `PreSonusScenePicker.gppanel` into the same **Extensions**
folder as the DLL. The extension loads that file instead of the built-in template when
present. **Do not rename** the GPScript widget handles (see checklist below).

## Presentation mode

| Widget | Visible when collapsed |
| ------ | ---------------------- |
| `CurrentSceneLabel` | Yes — friendly scene name |
| `SelectButton` | Yes |
| Picker widgets (`PickerProjectLabel`, prev/next, Recall, Done, …) | Hidden until Select |
| `ProjectStore`, `SceneStore` | Always hidden (persistence knobs) |

In the widget inspector for **ProjectStore** and **SceneStore**: **Ignore variations = OFF**.

## First load behavior

On rackspace **Initialization** the script:

1. Waits briefly for extension auto-connect (up to ~5 s).
2. Calls `GetProjectCount()` — may block **~5 s** while project/scene lists download.
3. Loads stored indices from `ProjectStore` / `SceneStore`.
4. Updates the scene label and **recalls** the stored scene to the desk.

GP may appear frozen briefly; this is expected once per rackspace load.

## Variations

- Browse with ◀ ▶ and tap **Recall** to commit a scene for the current variation.
- **Done** closes the picker; indices already saved via ◀ ▶ remain stored.
- Switching variations runs **auto-recall** for that variation’s stored scene.

Save the gig file to persist variation selections.

## Widget GPScript names (must match)

| Name | Type |
| ---- | ---- |
| `CurrentSceneLabel` | Label |
| `SelectButton` | Button |
| `PickerProjectLabel` | Label |
| `PickerSceneLabel` | Label |
| `ProjPrev`, `ProjNext` | Button |
| `ScenePrev`, `SceneNext` | Button |
| `ProjCountLabel`, `SceneCountLabel` | Label |
| `RecallButton`, `DoneButton` | Button |
| `ProjectStore`, `SceneStore` | Knob (hidden) |

## Smoke test

| Step | Expected |
| ---- | ---------- |
| Load gig | Strip shows friendly scene name; desk recalls stored scene |
| Select | Picker opens |
| Browse + Recall | Desk scene changes; strip updates; picker closes |
| Done (after browse) | Picker closes; desk unchanged since last recall |
| Variation A → B | Label and desk follow each variation |
| Save + reload gig | Variations retain scenes |

## Troubleshooting

| Symptom | Check |
| ------- | ----- |
| `(not connected)` | Mixer powered; auto-connect config; firewall |
| `(no project)` / count 0 | Connected? Wait for catalog load; see `extension.log` |
| Picker widgets visible at start | Script `Initialization` calls `SetPickerVisible(False)` |
| Variation does not stick | Store knobs must not use **Ignore variations** |
| Script compile errors on widget names | Panel template GPScript names must match exactly |

## Related APIs

- `PreSonusStudioLive_GetSceneDisplayName(sceneFile)` — friendly strip label
- `PreSonusStudioLive_GetProjectDisplayName(projectFile)` — friendly project name in picker
- `PreSonusStudioLive_RecallProjectScene(projectFile, sceneFile)`
