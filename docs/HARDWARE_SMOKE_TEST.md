# Hardware smoke test — StudioLive 32R

Run this on the **same PC as Gig Performer**, with the **32R powered on** and reachable on the LAN (TCP port **53000**).

## 1. Install the Release DLL

```powershell
cd C:\Users\KenHa\source\repos\presonus\presonus-studiolive-gp-extension
cmake --build build-rel --config Release --parallel
```

Copy `build-rel\bin\Release\PreSonusStudioLive.dll` into your GP **Extensions** folder, or run:

```powershell
.\tools\install-gp-release.ps1
```

Typical Extensions paths:

- `C:\Users\Public\Documents\Gig Performer\Extensions\`
- `%USERPROFILE%\Documents\Gig Performer\Extensions\`

In GP: **Options → Reload Third Party Libraries** (or restart GP). Enable **PreSonus StudioLive** under **Options → Extensions**.

## 2. Pre-flight

| Check | How |
| ----- | --- |
| Mixer IP | UC Surface / router DHCP list / ping the 32R |
| Port 53000 | `Test-NetConnection -ComputerName <ip> -Port 53000` |
| Channel | Use **input 1** as a **LINE** source (same as fixture capture) |
| No other client | Close UC Surface / other apps using the mixer TCP session |

## 3. GPScript (rackspace script)

GPScript rule: functions that **return a value** must be used in an assignment or inside
`Print(...)` — they cannot stand alone as a statement. `Disconnect()` has no return
value and may be called by itself.

Replace `YOUR_MIXER_IP` with the 32R address (e.g. `10.0.0.14`).

```gigperformer
// All-in-one smoke test (only valid top-level blocks: Var, Initialization, On …, Function)
Initialization
    Print(PreSonusStudioLive_SetLogLevel("debug"))
    Print(PreSonusStudioLive_LogFilePath())
    Print(PreSonusStudioLive_Version())
    Print(PreSonusStudioLive_Connect("10.0.0.14"))
    Print(PreSonusStudioLive_IsConnected())
    Print(PreSonusStudioLive_SetLineMute(1, 1))
    Print(PreSonusStudioLive_GetLineMute(1))
    Print(PreSonusStudioLive_SetLineMute(1, 0))
    Print(PreSonusStudioLive_GetLineMute(1))
End
```

**Optional — drive steps from a rackspace widget** (add a widget named `PSL_Test` in the
panel, then use this script):

```gigperformer
var PSL_Test : Widget

On WidgetValueChanged(newValue : Double) from PSL_Test
    if newValue < 0.25
        Print(PreSonusStudioLive_Connect("10.0.0.14"))
        Print(PreSonusStudioLive_IsConnected())
    elsif newValue < 0.5
        Print(PreSonusStudioLive_SetLineMute(1, 1))
    elsif newValue < 0.75
        Print(PreSonusStudioLive_SetLineMute(1, 0))
    else
        PreSonusStudioLive_Disconnect()
        Print("disconnected")
    end
End
```

Turn the widget in steps (or use a knob): low → connect, mid → mute on/off, high → disconnect.
Names like `OnUnmute` or `OnConnect` are **not** valid GPScript — only documented `On …` callbacks.

**Minimal one-shot** (paste into Initialization only — edit IP first):

```gigperformer
Initialization
    Print(PreSonusStudioLive_SetLogLevel("debug"))
    Print(PreSonusStudioLive_LogFilePath())
    Print(PreSonusStudioLive_Connect("10.0.0.14"))
    Print(PreSonusStudioLive_IsConnected())
    Print(PreSonusStudioLive_SetLineMute(1, 1))
End
```

> GPScript may not support `Wait()` in all contexts; prefer separate widget/button handlers if Initialization runs too fast.

### Phase 2 — unverified APIs (ready to paste)

Covers **level, solo, pan, color, project/scene list**, and optional **scene recall**.
Mute is included as a baseline (already verified on hardware). Uses **input 1** as LINE.

Edit `10.0.0.14` if your 32R is on a different address. The first
`GetProjectCount()` call **blocks the GP script thread for several seconds** while FD
file lists arrive — the UI may look frozen briefly; that is expected.

```gigperformer
// Phase 2 hardware smoke — LINE level/solo/pan/color + project/scene list
Var projectFile : String
Var sceneFile : String

Initialization
    Print(PreSonusStudioLive_SetLogLevel("debug"))
    Print(PreSonusStudioLive_LogFilePath())
    Print(PreSonusStudioLive_Version())
    Print(PreSonusStudioLive_Connect("10.0.0.14"))
    Print(PreSonusStudioLive_IsConnected())

    // Mute baseline (verified 2026-05-20)
    Print(PreSonusStudioLive_SetLineMute(1, 1))
    Print(PreSonusStudioLive_GetLineMute(1))
    Print(PreSonusStudioLive_SetLineMute(1, 0))
    Print(PreSonusStudioLive_GetLineMute(1))

    // Level — linear 0..100 (percent of fader travel)
    Print(PreSonusStudioLive_SetLineLevelLinear(1, 75.0))
    Print(PreSonusStudioLive_GetLineLevelLinear(1))
    Print(PreSonusStudioLive_SetLineLevelLinear(1, 25.0))
    Print(PreSonusStudioLive_GetLineLevelLinear(1))
    Print(PreSonusStudioLive_SetLineLevelLinear(1, 50.0))
    Print(PreSonusStudioLive_GetLineLevelLinear(1))

    // Solo — 1 = on, 0 = off (solo key not in capture fixtures; verify on desk)
    Print(PreSonusStudioLive_SetLineSolo(1, 1))
    Print(PreSonusStudioLive_GetLineSolo(1))
    Print(PreSonusStudioLive_SetLineSolo(1, 0))
    Print(PreSonusStudioLive_GetLineSolo(1))

    // Pan — 0 = full left, 50 = center, 100 = full right
    Print(PreSonusStudioLive_SetLinePan(1, 0.0))
    Print(PreSonusStudioLive_GetLinePan(1))
    Print(PreSonusStudioLive_SetLinePan(1, 100.0))
    Print(PreSonusStudioLive_GetLinePan(1))
    Print(PreSonusStudioLive_SetLinePan(1, 50.0))
    Print(PreSonusStudioLive_GetLinePan(1))

    // Color — RRGGBB or #RRGGBB
    Print(PreSonusStudioLive_SetLineColor(1, "FF0000"))
    Print(PreSonusStudioLive_GetLineColor(1))
    Print(PreSonusStudioLive_SetLineColor(1, "0000FF"))
    Print(PreSonusStudioLive_GetLineColor(1))

    // Project / scene catalog (1-based indexes; blocks on first list fetch)
    Print(PreSonusStudioLive_GetProjectCount())
    projectFile = PreSonusStudioLive_GetProjectName(1)
    Print(projectFile)
    Print(PreSonusStudioLive_GetSceneCount(projectFile))
    sceneFile = PreSonusStudioLive_GetSceneName(projectFile, 1)
    Print(sceneFile)

    // Optional — loads scene 1 of project 1 on the desk (uncomment to test recall)
    // Print(PreSonusStudioLive_RecallProjectScene(projectFile, sceneFile))
End
```

**If string assignment fails** (`projectFile = PreSonusStudioLive_GetProjectName(1)`), read
project/scene file names from the GP log after `GetProjectCount()`, then call the scene APIs
with literals from a widget handler, e.g.
`Print(PreSonusStudioLive_GetSceneCount("01.West End Girls.proj"))` (names vary by mixer).

**Stepped widget version** — add a knob/slider named `PSL_Phase2`, then paste:

```gigperformer
Var PSL_Phase2 : Widget
Var projectFile : String
Var sceneFile : String

On WidgetValueChanged(newValue : Double) from PSL_Phase2
    if newValue < 0.1
        Print(PreSonusStudioLive_Connect("10.0.0.14"))
        Print(PreSonusStudioLive_IsConnected())
    elsif newValue < 0.2
        Print(PreSonusStudioLive_SetLineLevelLinear(1, 75.0))
        Print(PreSonusStudioLive_GetLineLevelLinear(1))
    elsif newValue < 0.3
        Print(PreSonusStudioLive_SetLineLevelLinear(1, 25.0))
    elsif newValue < 0.4
        Print(PreSonusStudioLive_SetLineSolo(1, 1))
        Print(PreSonusStudioLive_GetLineSolo(1))
    elsif newValue < 0.5
        Print(PreSonusStudioLive_SetLineSolo(1, 0))
    elsif newValue < 0.6
        Print(PreSonusStudioLive_SetLinePan(1, 0.0))
        Print(PreSonusStudioLive_GetLinePan(1))
    elsif newValue < 0.7
        Print(PreSonusStudioLive_SetLinePan(1, 100.0))
        Print(PreSonusStudioLive_SetLinePan(1, 50.0))
    elsif newValue < 0.8
        Print(PreSonusStudioLive_SetLineColor(1, "FF0000"))
        Print(PreSonusStudioLive_GetLineColor(1))
    elsif newValue < 0.9
        Print(PreSonusStudioLive_GetProjectCount())
        projectFile = PreSonusStudioLive_GetProjectName(1)
        Print(projectFile)
        Print(PreSonusStudioLive_GetSceneCount(projectFile))
        sceneFile = PreSonusStudioLive_GetSceneName(projectFile, 1)
        Print(sceneFile)
    else
        Print(PreSonusStudioLive_RecallProjectScene(projectFile, sceneFile))
    end
End
```

Turn the widget slowly in bands (0–10%, 10–20%, …) so you can watch the desk between steps.
The top band recalls a scene — only use it when you are ready for the loaded scene to change.

## 4. What to observe

### Gig Performer log

Expect lines similar to:

- `PSL version: 1.0.0-phase0`
- `Connect(...) = true` / `IsConnected = true`
- `SetLineMute(1, mute) = true` (boolean only means the command was **queued/sent**, not that the desk changed)

### File log

Open the path from `PreSonusStudioLive_LogFilePath()`, usually:

`%APPDATA%\PreSonusStudioLive\extension.log`

Look for session banner, `Mixer connected to <ip>`, and any warnings.

### Mixer surface

Watch **input 1** on the desk or in UC Surface:

| API | What should move |
| --- | ---------------- |
| `SetLineMute` | Mute LED / mute button state |
| `SetLineLevelLinear` | Channel 1 fader (75% → 25% → 50%) |
| `SetLineSolo` | Solo LED / solo bus indicator |
| `SetLinePan` | Pan knob (hard left → hard right → center) |
| `SetLineColor` | Channel color strip (red → blue) |
| `RecallProjectScene` | Loaded project/scene title changes |

For project/scene list APIs, confirm the GP log shows a **non-zero** `GetProjectCount`, sensible `GetProjectName(1)` (e.g. `01.West End Girls.proj`), and matching scene file names. **`GetSceneName(1)` must be a `.scn` file** (e.g. `01.Live Performance.scn`), not a `.cnfg` project config.
Empty strings or `0` counts usually mean FD list fetch failed — check `extension.log`.

`RecallProjectScene` returning **True** only means the JM RestorePreset packet was **sent**; confirm the loaded scene title changes on the desk. After recall, check `extension.log` for `Recall scene JM RestorePreset: presets/proj/...`.

## 5. Record results

### Connection (verified 2026-05-20)

| Step | Pass? | Notes |
| ---- | ----- | ----- |
| Extension loads in GP | | |
| `Version()` prints | | |
| `Connect` → true | | |
| `IsConnected` → true | | |
| `extension.log` has connect line | | |
| Input 1 mute **changes on desk** | | |
| `SetLineMute` returns true | | |

### Phase 2 — unverified (fill in after paste block above)

| Step | Pass? | Desk moved? | Notes |
| ---- | ----- | ----------- | ----- |
| `SetLineLevelLinear` / `GetLineLevelLinear` | | | Fader at 75 / 25 / 50 |
| `SetLineSolo` / `GetLineSolo` | | | Solo on then off |
| `SetLinePan` / `GetLinePan` | | | L / R / center |
| `SetLineColor` / `GetLineColor` | | | Red then blue |
| `GetProjectCount` > 0 | | n/a | GP may pause ~5 s |
| `GetProjectName(1)` non-empty | | n/a | |
| `GetSceneCount` > 0 | | n/a | |
| `GetSceneName(1)` non-empty | | n/a | |
| `RecallProjectScene` (optional) | | | Loaded scene changed |

**If connect works but mute does not move the desk:** rebuild with a DLL that includes UCNet handshake (JM Subscribe + wait for ZB + `SubscriptionReply`). Older builds only opened TCP without subscribing.

**If `RecallProjectScene` returns True but the desk does not change:** confirm `GetSceneName(1)` ends in `.scn` (not `.cnfg`). Check `extension.log` for `Recall scene JM RestorePreset:` (not the old FR Open path).

**If connect fails:** verify IP, firewall, and that nothing else holds the TCP session.

## 6. After the test

Paste into your session notes (or an issue):

- GP version (5 Pro / etc.)
- 32R firmware (e.g. 3.3.0.109659)
- Mixer IP
- Connect true/false
- Mute / level / solo / pan / color — desk yes/no for each
- Project count and first project/scene names from GP log
- Scene recall true/false (if tested)
- Last ~30 lines of `extension.log`

## 7. Widget binding (Phase 3)

Create two panel widgets before running this section:

| Widget name | Type | Range |
| ----------- | ---- | ----- |
| `PSL_Fader` | Knob or slider | 0..1 |
| `PSL_Mute` | **Switch** (toggle) | 0..1 |

Use a **Switch**, not a momentary **Button** — buttons do not expose a stable value to the extension API and `BindLineMuteWidget` will return **False**.

Reload the DLL, then paste (widget names are **strings** — no `Var … : Widget` needed for bind calls):

```gigperformer
Initialization
    SetTimersRunning(true)
    Print(PreSonusStudioLive_SetLogLevel("debug"))
    Print(PreSonusStudioLive_Connect("10.0.0.14"))
    Print(PreSonusStudioLive_BindLineLevelWidgetLinear("PSL_Fader", 1, 2))
    Print(PreSonusStudioLive_BindLineMuteWidget("PSL_Mute", 1, 2))
End

On TimerTick(milliseconds : Double)
    Print(PreSonusStudioLive_PollWidgetBindings())
End
```

**Mixer→widget polling:** Incoming desk changes are queued on the IO thread and applied when the GP thread drains. GP has no extension idle/timer callback in SDK v62, so rackspaces with `direction` **0** or **2** must call `PollWidgetBindings()` periodically — typically from **`On TimerTick`** after `SetTimersRunning(true)`. Widget→desk (`direction` **1** or **2**) does not need the timer; it uses `OnWidgetValueChanged`. A future version could hide the timer if Gig Performer adds a periodic extension callback.

`direction`: **0** = mixer→widget only, **1** = widget→mixer only, **2** = both (recommended).

### What to observe

| Step | Pass? | Notes |
| ---- | ----- | ----- |
| Both `Bind*` calls return **True** | | False → widget name mismatch or widget missing from panel |
| Move **desk** input 1 fader → `PSL_Fader` follows | | May lag one GP script tick |
| Move **`PSL_Fader`** → desk fader follows | | |
| Toggle **`PSL_Mute`** high → input 1 mutes on desk | | Uses >= 0.5 as muted |
| Toggle desk mute → `PSL_Mute` follows | | |
| `extension.log` shows PV traffic | | No errors |

**If rackspace fails to compile with `Unexpected token: 'widget'`:** older DLL builds registered a GPScript parameter named `widget`, which clashes with GPScript's `Widget` type. Rebuild/install the latest DLL (parameter renamed to `widgetName`).

**If desk→widget never updates:** bindings only poll when GP calls an extension function or `OnWidgetValueChanged` fires. Move the desk, then touch any bound widget or add a slow widget handler to force polls.

**If widget→desk does not work:** confirm `direction` is **1** or **2** and the extension registered `OnWidgetValueChanged` (reload libraries after DLL update).

## 8. AUX / FX send levels (Phase 2)

Route **input 1** to **AUX 1** in UC Surface first (fixture `06-pv-aux-send-level` was skipped when ch1 was not routed).

```gigperformer
Initialization
    Print(PreSonusStudioLive_Connect("10.0.0.14"))
    Print(PreSonusStudioLive_SetLevelLinear("LINE", 1, "AUX", 1, 75.0, 0))
    Print(PreSonusStudioLive_GetLevelLinear("LINE", 1, "AUX", 1))
    Print(PreSonusStudioLive_SetMute("LINE", 1, "AUX", 1, 0))
    Print(PreSonusStudioLive_GetMute("LINE", 1, "AUX", 1))
    Print(PreSonusStudioLive_SetLevelLinear("LINE", 1, "FX", 1, 50.0, 0))
End
```

For **main mix**, pass empty mix type and mix number **0** (`fadeMs` **0** = instant):

```gigperformer
Print(PreSonusStudioLive_SetLevelLinear("LINE", 1, "", 0, 50.0, 0))
```

| Step | Pass? | Desk moved? | Notes |
| ---- | ----- | ----------- | ----- |
| AUX 1 send fader → 75% | | | Key `line/ch1/aux1` (lowercase; session capture) |
| AUX 1 send un-muted (`SetMute(..., 0)`) | | | Uses `assign_aux1` (inverted) |
| FX 1 send fader → 50% | | | Key `line/ch1/FXA` |

## 9. Song → scene binding (Phase 3)

**Status:** **hardware-verified 2026-05-22** on 32R @ `10.0.0.14`.

List projects/scenes first (§6), then bind GP setlist song index **0** to a known scene.
Recall is driven by the extension C++ `OnSongChanged` callback — **no GPScript `On Song` handler required**
(put `BindSongToScene` in rackspace `Initialization` only).

**Setlist mode required.** Switch songs from the GP setlist UI (not rackspace variations). If you are
already on song **0**, switch to another song first, then back to **0** — `OnSongChanged` fires only
on a change.

```gigperformer
Initialization
    Print(PreSonusStudioLive_Connect("10.0.0.14"))
    Print(PreSonusStudioLive_BindSongToScene(0, "01.West End Girls.proj", "01.Live Performance.scn"))
End
```

| Step | Pass? | Notes |
| ---- | ----- | ----- |
| `BindSongToScene` returns **True** | | Project/scene filenames must match FD list exactly |
| Change GP setlist to song **0** | | Desk recalls **Live Performance** scene |
| Change to another song (unbound) | | Desk unchanged |

## 10. Fade transitions (Phase 2)

**Status:** **hardware-verified 2026-05-22** on 32R @ `10.0.0.14`.

Fades run on the mixer IO thread (ease-in-out sine, ~10 ms steps). Pass **`fadeMs`** as the last argument; **0** = instant (same as before).

```gigperformer
Initialization
    Print(PreSonusStudioLive_Connect("10.0.0.14"))
    Print(PreSonusStudioLive_SetLevelLinear("LINE", 1, "", 0, 0.0, 0))
    Print(PreSonusStudioLive_SetLevelLinear("LINE", 1, "", 0, 75.0, 500))
End
```

| Step | Pass? | Desk moved? | Notes |
| ---- | ----- | ----------- | ----- |
| Instant set to 0% (`fadeMs` 0) | | | |
| 500 ms fade 0% → 75% | | | Smooth fader motion; multiple PV packets in log |
| dB fade works | | | `SetLevelDb("LINE", 1, "", 0, 0.0, 500)` from current level |

## 11. UDP discovery (Phase 4)

**Status:** **hardware-verified 2026-05-22** on 32R @ `10.0.0.14` (serial `RA3E18090022`).

Mixers broadcast UCNet discovery on UDP port **47809** every ~3 s. The GP PC must be on the **same LAN broadcast domain** as the mixer (routed subnets see TCP to a known IP but not UDP discovery). Close UC Surface if it holds exclusive resources. Allow UDP **47809** through Windows firewall on the GP PC.

On multi-homed PCs you may see duplicate `Discovery:` log lines for the same serial from `127.0.0.1` or secondary interface IPs — dedupe is by serial; `GetDiscoveredHost(1)` returns one entry per mixer.

```gigperformer
Initialization
    Print(PreSonusStudioLive_SetLogLevel("debug"))
    Print(PreSonusStudioLive_Discover(5000))
    Print(PreSonusStudioLive_GetDiscoveredHost(1))
    Print(PreSonusStudioLive_GetDiscoveredName(1))
    Print(PreSonusStudioLive_GetDiscoveredSerial(1))
    Print(PreSonusStudioLive_DiscoverAndConnect(5000))
    Print(PreSonusStudioLive_IsConnected())
    Print(PreSonusStudioLive_GetConnectedHost())
    Print(PreSonusStudioLive_GetConnectedName())
End
```

| Step | Pass? | Notes |
| ---- | ----- | ----- |
| `Discover(5000)` > 0 | | At least one mixer on LAN |
| `GetDiscoveredHost(1)` matches desk IP | | 1-based index |
| `GetDiscoveredSerial(1)` non-empty | | e.g. `RA3E18090022` on test 32R |
| `DiscoverAndConnect(5000)` → **True** | | Prefers last serial/host in `%APPDATA%\PreSonusStudioLive\config.json` |
| `GetConnectedHost()` / `GetConnectedName()` | | Populated after connect |
| `extension.log` | | Lines `Discovery: …` then `Mixer connected to …` |

**Alternative:** keep using `Connect("10.0.0.14")` — successful connects still update `config.json` (`lastHost`, `lastSerial`, `lastMixerName`).
