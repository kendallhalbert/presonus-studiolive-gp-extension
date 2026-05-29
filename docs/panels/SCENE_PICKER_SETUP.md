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

1. Build + install: `cmake --build build-rel --config Release` then `.\tools\install-gp-release.ps1`
2. **Options → Reload Third Party Libraries** and enable **PreSonus StudioLive**.

> **Panel template disabled:** `GetPanelXML` panel insertion crashes Gig Performer 5.2.2
> with every XML format we have tested. Build the panel **manually** in GP (steps below).
> The extension GPScript and mixer APIs work without the template.

## Build the panel manually in GP

1. Open a rackspace in **Panels** view → **Edit** mode.
2. Create a **new empty panel** (or use an existing panel strip).
3. Add widgets and set **Widget Properties → Advanced → Widget handle** for each:

| Handle | Widget type | Visible when collapsed | Notes |
| ------ | ----------- | ---------------------- | ----- |
| `CurrentSceneLabel` | Text Label | Yes | Friendly scene name (script sets text) |
| `SelectButton` | Button | Yes | Opens picker |
| `PickerProjectLabel` | Text Label | Hidden | Project name in picker |
| `ProjPrev` | Button | Hidden | ◀ project |
| `ProjNext` | Button | Hidden | ▶ project |
| `ProjCountLabel` | Text Label | Hidden | e.g. `1/3` |
| `PickerSceneLabel` | Text Label | Hidden | Scene name in picker |
| `ScenePrev` | Button | Hidden | ◀ scene |
| `SceneNext` | Button | Hidden | ▶ scene |
| `SceneCountLabel` | Text Label | Hidden | e.g. `2/5` |
| `RecallButton` | Button | Hidden | Commits scene to desk |
| `DoneButton` | Button | Hidden | Closes picker |
| `ProjectStore` | Knob | Hidden | **Ignore variations = OFF** |
| `SceneStore` | Knob | Hidden | **Ignore variations = OFF** |

4. Set **Hide in Presentation View** on all picker widgets (everything except
   `CurrentSceneLabel` and `SelectButton`).
5. Open **Window → Show Rackspace Script Editor**.
6. Paste [`scene-picker-collapsed.gpscript`](scene-picker-collapsed.gpscript).
7. Save the gig.

### Tips

- After placing widgets, use **Presentation** mode preview to confirm only the strip
  (label + Select) shows when collapsed.
- Store knobs (`ProjectStore`, `SceneStore`) can be tiny and tucked in a corner — the
  script never shows them to the performer.
- Export your finished panel (**File → Export panel** or GP’s panel export) and keep a
  copy in the gig file; you can reuse it on other rackspaces.

## Presentation mode

| Widget | Visible when collapsed |
| ------ | ---------------------- |
| `CurrentSceneLabel` | Yes — friendly scene name |
| `SelectButton` | Yes |
| Picker widgets (`PickerProjectLabel`, prev/next, Recall, Done, …) | Hidden until Select |
| `ProjectStore`, `SceneStore` | Always hidden (persistence knobs) |

In the widget inspector for **ProjectStore** and **SceneStore**: **Ignore variations = OFF**.

## First load behavior

GPScript callbacks cannot block (there is no `Wait`/`Sleep`), so the script shows
 **`(connecting...)`** on load, enables timers, and defers the first recall to
`On TimerTick`. As soon as the extension auto-connect completes, the timer runs the
initial load **once**:

1. Calls `GetProjectCount()` — may pause briefly while project/scene lists download.
2. Loads stored indices from `ProjectStore` / `SceneStore`.
3. Updates the scene label and **recalls** the stored scene to the desk.

This needs **`SetTimersRunning(true)`** (the script calls it in `Initialization`). If you
also poll meters in the same rackspace, the shared `On TimerTick` is fine.

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
| Script compile errors on widget names | Widget handles in Advanced tab must match exactly |
| GP crash on “New panel → PreSonus Scene Picker” | Template is disabled (`GetPanelCount=0`); build panel manually |

## Related APIs

- `PreSonusStudioLive_GetSceneDisplayName(sceneFile)` — friendly strip label
- `PreSonusStudioLive_GetProjectDisplayName(projectFile)` — friendly project name in picker
- `PreSonusStudioLive_RecallProjectScene(projectFile, sceneFile)`
