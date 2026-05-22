# PreSonus StudioLive — Gig Performer Extension

Native Windows extension (`PreSonusStudioLive.dll`) that connects
[Gig Performer](https://gigperformer.com) to a **PreSonus StudioLive III**
mixer over UCNet (TCP port 53000). Control the desk from **GPScript** using
`PreSonusStudioLive_*` functions — connect, channel parameters (LINE, RETURN, DCA, SUB,
AUX/FX sends, buses), project/scene/preset lists, scene recall, widget mirroring, and
UDP discovery with auto-connect.

**Independent third-party project** — not affiliated with, endorsed by, or sponsored
by PreSonus Audio Electronics, Inc. or Gig Performer.

This repo is the C++ implementation that supersedes the Node.js
[`@featherbear/presonus-studiolive-api`](https://github.com/featherbear/presonus-studiolive-api)
library. Wire-format behavior is validated against captured fixtures from a
**StudioLive 32R** (firmware 3.3.0.109659).

> **Status (2026-05-22):** Phases 0–4 **complete**; Phase 2 generic channel types **§14 verified** on 32R @ `10.0.0.14`.
> **72 unit tests** across 28 executables. Full hardware smoke test **§1–§14** in `docs/HARDWARE_SMOKE_TEST.md`.
> **Next optional:** Phase 5 metering.
>
> Design, roadmap, and phase detail:
> [`docs/GP_EXTENSION_PLAN.md`](docs/GP_EXTENSION_PLAN.md).
> Mixer on-desk validation:
> [`docs/HARDWARE_SMOKE_TEST.md`](docs/HARDWARE_SMOKE_TEST.md).

---

## What works today

| Area | GPScript surface | Notes |
| ---- | ---------------- | ----- |
| Session | `Connect`, `Disconnect`, `IsConnected`, `GetConnectedHost`, `GetConnectedName` | TCP port 53000; config saved on success |
| Discovery | `Discover`, `GetDiscoveredHost/Name/Serial`, `DiscoverAndConnect` | UDP listen on **47809**; prefers last serial/host from config; **auto-connect on gig load** |
| Logging | `SetLogLevel`, `LogFilePath` | File log under `%APPDATA%\PreSonusStudioLive\extension.log` |
| LINE inputs | Mute, level (0–100%), solo, pan, color | 1-based channel; LINE shortcuts have no `fadeMs` (instant only) |
| Generic channels | `GetChannelCount`, `SetMute`, `GetMute`, `ToggleMute`, `SetLevelLinear/Db`, `GetLevelLinear/Db`, `SetSolo`, `GetSolo`, `ToggleSolo`, `SetPan`, `GetPan`, `SetColor`, `GetColor` | `type` = `"LINE"`, `"RETURN"`, `"DCA"`, `"SUB"`, `"MAIN"`, `"AUX"`, `"FX"`, etc.; **`fadeMs` required** on set-level (0 = instant) |
| Projects / scenes | `GetProjectCount`, `GetProjectName`, `GetSceneCount`, `GetSceneName`, `RecallProjectScene`, `GetCurrentProject`, `GetCurrentScene` | First list call can block ~5s while FD payloads arrive |
| Channel presets | `GetChannelPresetCount`, `GetChannelPresetName`, `RecallChannelStrip` | List under `presets/channel/`; recall enables desk filter flags before JM `RestorePreset` |
| Widget bindings | LINE shortcuts + generic `BindLevelWidgetLinear/Db`, `BindMuteWidget`, `BindSoloWidget`, `UnbindWidget`, `UnbindAll`, `PollWidgetBindings` | Mute bind needs a **Switch** widget; call `PollWidgetBindings` from `On TimerTick` for desk→widget |
| Song → scene | `BindSongToScene`, `BindSongPartToScene`, `UnbindSong` | Recalls scene on GP setlist change (`OnSongChanged`) |
| Meta | `Version` | Currently reports `1.0.0-phase0` |

**Planned (not in this build):** real-time meters (`SubscribeMeters`, `GetMeterLevel`) — see the plan doc §5.

**Tested host:** Gig Performer **5 Pro** with GP SDK **`beta-sdk-v62`** (SDK version 62).
GP 5 rejects extensions built against older SDK branches.

---

## GPScript API

Gig Performer builds function names from the product XML `Name` attribute (capital
**`Name=`** — lowercase `name=` is ignored on GP 5). Spaces are stripped, so
`Name="PreSonus StudioLive"` yields the prefix **`PreSonusStudioLive_`**.

C++ entry points use a `psl_` prefix internally; only the GPScript names below are
visible to script authors.

| Function | Arguments | Returns |
| -------- | --------- | ------- |
| `PreSonusStudioLive_Version` | — | String |
| `PreSonusStudioLive_Connect` | `host : String` | Boolean |
| `PreSonusStudioLive_Disconnect` | — | — |
| `PreSonusStudioLive_IsConnected` | — | Boolean |
| `PreSonusStudioLive_GetConnectedHost` | — | String |
| `PreSonusStudioLive_GetConnectedName` | — | String |
| `PreSonusStudioLive_Discover` | `timeoutMs : Integer` | Integer |
| `PreSonusStudioLive_GetDiscoveredHost` | `index : Integer` | String |
| `PreSonusStudioLive_GetDiscoveredName` | `index : Integer` | String |
| `PreSonusStudioLive_GetDiscoveredSerial` | `index : Integer` | String |
| `PreSonusStudioLive_DiscoverAndConnect` | `timeoutMs : Integer` | Boolean |
| `PreSonusStudioLive_SetLogLevel` | `level : String` | Boolean |
| `PreSonusStudioLive_LogFilePath` | — | String |
| `PreSonusStudioLive_SetLineMute` | `channel : Integer`, `muted : Integer` | Boolean |
| `PreSonusStudioLive_GetLineMute` | `channel : Integer` | Boolean |
| `PreSonusStudioLive_SetLineLevelLinear` | `channel : Integer`, `level : Double` | Boolean |
| `PreSonusStudioLive_GetLineLevelLinear` | `channel : Integer` | Double |
| `PreSonusStudioLive_SetLineSolo` | `channel : Integer`, `soloed : Integer` | Boolean |
| `PreSonusStudioLive_GetLineSolo` | `channel : Integer` | Boolean |
| `PreSonusStudioLive_SetLinePan` | `channel : Integer`, `pan : Double` | Boolean |
| `PreSonusStudioLive_GetLinePan` | `channel : Integer` | Double |
| `PreSonusStudioLive_SetLineColor` | `channel : Integer`, `rgbHex : String` | Boolean |
| `PreSonusStudioLive_GetLineColor` | `channel : Integer` | String |
| `PreSonusStudioLive_GetProjectCount` | — | Integer |
| `PreSonusStudioLive_GetProjectName` | `index : Integer` | String |
| `PreSonusStudioLive_GetSceneCount` | `projectFile : String` | Integer |
| `PreSonusStudioLive_GetSceneName` | `projectFile : String`, `index : Integer` | String |
| `PreSonusStudioLive_RecallProjectScene` | `projectFile : String`, `sceneFile : String` | Boolean |
| `PreSonusStudioLive_GetCurrentProject` | — | String |
| `PreSonusStudioLive_GetCurrentScene` | — | String |
| `PreSonusStudioLive_GetChannelPresetCount` | — | Integer |
| `PreSonusStudioLive_GetChannelPresetName` | `index : Integer` | String |
| `PreSonusStudioLive_RecallChannelStrip` | `type : String`, `channel : Integer`, `chanFile : String` | Boolean |
| `PreSonusStudioLive_GetChannelCount` | `type : String` | Integer |
| `PreSonusStudioLive_SetMute` | `type : String`, `channel : Integer`, `mixType : String`, `mixNumber : Integer`, `muted : Integer` | Boolean |
| `PreSonusStudioLive_GetMute` | `type : String`, `channel : Integer`, `mixType : String`, `mixNumber : Integer` | Boolean |
| `PreSonusStudioLive_ToggleMute` | `type : String`, `channel : Integer`, `mixType : String`, `mixNumber : Integer` | Boolean |
| `PreSonusStudioLive_SetLevelLinear` | `type : String`, `channel : Integer`, `mixType : String`, `mixNumber : Integer`, `level : Double`, `fadeMs : Integer` | Boolean |
| `PreSonusStudioLive_GetLevelLinear` | `type : String`, `channel : Integer`, `mixType : String`, `mixNumber : Integer` | Double |
| `PreSonusStudioLive_SetLevelDb` | `type : String`, `channel : Integer`, `mixType : String`, `mixNumber : Integer`, `db : Double`, `fadeMs : Integer` | Boolean |
| `PreSonusStudioLive_GetLevelDb` | `type : String`, `channel : Integer`, `mixType : String`, `mixNumber : Integer` | Double |
| `PreSonusStudioLive_SetSolo` | `type : String`, `channel : Integer`, `soloed : Integer` | Boolean |
| `PreSonusStudioLive_GetSolo` | `type : String`, `channel : Integer` | Boolean |
| `PreSonusStudioLive_ToggleSolo` | `type : String`, `channel : Integer` | Boolean |
| `PreSonusStudioLive_SetPan` | `type : String`, `channel : Integer`, `pan : Double` | Boolean |
| `PreSonusStudioLive_GetPan` | `type : String`, `channel : Integer` | Double |
| `PreSonusStudioLive_SetColor` | `type : String`, `channel : Integer`, `rgbHex : String` | Boolean |
| `PreSonusStudioLive_GetColor` | `type : String`, `channel : Integer` | String |
| `PreSonusStudioLive_BindLineLevelWidgetLinear` | `widgetName : String`, `channel : Integer`, `direction : Integer` | Boolean |
| `PreSonusStudioLive_BindLineLevelWidgetDb` | `widgetName : String`, `channel : Integer`, `direction : Integer` | Boolean |
| `PreSonusStudioLive_BindLineMuteWidget` | `widgetName : String`, `channel : Integer`, `direction : Integer` | Boolean |
| `PreSonusStudioLive_BindLineSoloWidget` | `widgetName : String`, `channel : Integer`, `direction : Integer` | Boolean |
| `PreSonusStudioLive_BindLevelWidgetLinear` | `widgetName : String`, `type : String`, `channel : Integer`, `mixType : String`, `mixNumber : Integer`, `direction : Integer` | Boolean |
| `PreSonusStudioLive_BindLevelWidgetDb` | `widgetName : String`, `type : String`, `channel : Integer`, `mixType : String`, `mixNumber : Integer`, `direction : Integer` | Boolean |
| `PreSonusStudioLive_BindMuteWidget` | `widgetName : String`, `type : String`, `channel : Integer`, `mixType : String`, `mixNumber : Integer`, `direction : Integer` | Boolean |
| `PreSonusStudioLive_BindSoloWidget` | `widgetName : String`, `type : String`, `channel : Integer`, `direction : Integer` | Boolean |
| `PreSonusStudioLive_UnbindWidget` | `widgetName : String` | Boolean |
| `PreSonusStudioLive_UnbindAll` | — | — |
| `PreSonusStudioLive_PollWidgetBindings` | — | Boolean |
| `PreSonusStudioLive_BindSongToScene` | `songIndex : Integer`, `projectFile : String`, `sceneFile : String` | Boolean |
| `PreSonusStudioLive_BindSongPartToScene` | `songIndex : Integer`, `partIndex : Integer`, `projectFile : String`, `sceneFile : String` | Boolean |
| `PreSonusStudioLive_UnbindSong` | `songIndex : Integer` | Boolean |

**Generic channel args:** `type` is `"LINE"`, `"RETURN"`, `"DCA"`, `"SUB"`, `"MAIN"`, `"AUX"`, `"FX"`, etc.
(DCA maps to wire type `filtergroup`; `"MAIN"` always uses channel 1). For **main mix** level/mute/solo/pan/color,
pass `mixType=""` and `mixNumber=0`. For **AUX 1** send: `mixType="AUX"`, `mixNumber=1`.
For **FX A** send: `mixType="FX"`, `mixNumber=1`. **`fadeMs` 0** = instant; **> 0** =
ease-in-out fade on the IO thread (~10 ms PV steps). **`GetChannelCount`** reads from the
post-handshake state cache (0 until connected).

**Widget `direction`:** `0` = desk→widget, `1` = widget→desk, `2` = both. For `0` or `2`,
call `PollWidgetBindings()` from `On TimerTick` (with `SetTimersRunning(true)`).

GPScript requires functions that return a value to appear inside `Print(...)` or an
assignment. `Disconnect()` and `UnbindAll()` may be called as standalone statements.

Minimal rackspace check:

```gigperformer
Initialization
    Print(PreSonusStudioLive_Version())
End
```

---

## Prerequisites

| Tool | Notes |
| ---- | ----- |
| Windows 10/11 x64 | macOS/Linux are out of scope for v1 |
| Visual Studio 2022 or 2026 | “Desktop development with C++” workload |
| CMake ≥ 3.21 | Bundled with VS; Ninja Multi-Config recommended |
| [Gig Performer SDK](https://github.com/gigperformer/gp-sdk) | Branch **`beta-sdk-v62`** required for GP 5 |
| Gig Performer ≥ 4.8 | Validated on **GP 5 Pro** |

Clone the SDK and check out the correct branch **before** configuring this project:

```powershell
git clone https://github.com/gigperformer/gp-sdk C:\path\to\gp-sdk
cd C:\path\to\gp-sdk
git checkout beta-sdk-v62
```

Pass your clone path to CMake with `-DGP_SDK_DIR=C:/path/to/gp-sdk`. The default
cache path in `CMakeLists.txt` is machine-specific; override it on every machine.

---

## Build

Use a **Developer PowerShell for VS** prompt so `cl` and Ninja are on `PATH`.

### Unit tests (Debug)

```powershell
cmake -S . -B build -G "Ninja Multi-Config" -DGP_SDK_DIR=C:/path/to/gp-sdk
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

Visual Studio generator alternative:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DGP_SDK_DIR=C:/path/to/gp-sdk
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

Debug output: `build/bin/Debug/PreSonusStudioLive.dll` — for local dev and CI only.

### Gig Performer install (Release)

Gig Performer **cannot** load a Debug build. Debug DLLs depend on `MSVCP140D.dll`,
`VCRUNTIME140D.dll`, and `ucrtbased.dll`, which are not present in a normal GP install.
GP will silently ignore the extension.

```powershell
cmake -S . -B build-rel -G "Visual Studio 17 2022" -A x64 -DGP_SDK_DIR=C:/path/to/gp-sdk
cmake --build build-rel --config Release --parallel
```

Release output: `build-rel/bin/Release/PreSonusStudioLive.dll`.

Install into Gig Performer’s Extensions folder (pick one method):

```powershell
# Helper: finds Public Documents, %USERPROFILE%\Documents, or OneDrive Extensions
.\tools\install-gp-release.ps1

# Or CMake install (default destination is Public Documents\Gig Performer\Extensions)
cmake --install build-rel --config Release --component dev
```

Typical Extensions paths:

- `C:\Users\Public\Documents\Gig Performer\Extensions\`
- `%USERPROFILE%\Documents\Gig Performer\Extensions\`

In Gig Performer: **Options → Reload Third Party Libraries** (or restart), then enable
**PreSonus StudioLive** under **Options → Extensions**. Expect version `1.0.0-phase0`.

---

## Hardware smoke test

With a StudioLive on the LAN (TCP **53000** open, UC Surface closed so only one client
holds the session), follow
[`docs/HARDWARE_SMOKE_TEST.md`](docs/HARDWARE_SMOKE_TEST.md) for a full GPScript
exercise (connect, mute, optional widget-driven steps, log file location).

Confirmed on **StudioLive 32R** @ `10.0.0.14`: connect, LINE controls, AUX/FX sends, widgets,
discovery, auto-connect, channel presets, and RETURN/DCA/SUB (§14).

---

## Repository layout

```
.
├── CMakeLists.txt                 # project + GP_SDK_DIR + install path
├── extension/
│   ├── CMakeLists.txt
│   └── src/
│       ├── LibMain.cpp            # GigPerformerAPI subclass, product XML
│       ├── bridge/                # GP host, dispatcher, ScriptFunctions, logging
│       ├── protocol/              # UCNet parsers, handshake, MixerConnection
│       ├── transport/             # WinSock + scripted transports
│       ├── state/                 # KvCache (PV/PS/PC/MS + ZB flatten)
│       └── mixer/                 # MixerService IO thread
├── tests/
│   ├── CMakeLists.txt             # one GoogleTest executable per unit/*.cpp
│   ├── fixtures/                  # captured wire traffic (32R, fw 3.3.0.109659)
│   └── unit/                      # protocol, transport, cache, LINE controls, …
├── docs/
│   ├── GP_EXTENSION_PLAN.md       # design + phased roadmap (canonical)
│   └── HARDWARE_SMOKE_TEST.md     # on-desk validation runbook
├── tools/
│   └── install-gp-release.ps1     # copy Release DLL into GP Extensions
├── .github/workflows/ci.yml       # Windows Debug build + ctest (clones beta-sdk-v62)
└── cmake/                         # helper modules (reserved)
```

---

## CI

GitHub Actions (`.github/workflows/ci.yml`) on `windows-2022`:

1. Clone `gp-sdk` at **`beta-sdk-v62`**
2. Configure with Ninja Multi-Config and `-DPSL_WARNINGS_AS_ERRORS=ON`
3. Debug build + `ctest -C Debug`
4. Upload `PreSonusStudioLive.dll` as a workflow artifact

Remote: https://github.com/kendallhalbert/presonus-studiolive-gp-extension

---

## Trademarks

PreSonus® and StudioLive® are registered trademarks of [PreSonus Audio
Electronics, Inc.](https://www.presonus.com/) UC Surface™ and UCNet™ are
trademarks of PreSonus Audio Electronics, Inc. Gig Performer® is a trademark
of Gig Performer LLC.

This project is an **independent third-party extension**. It is **not**
affiliated with, endorsed by, or sponsored by PreSonus Audio Electronics, Inc.
or Gig Performer LLC. Trademarks are used only to identify compatible products
and services (nominative fair use).

---

## License

MIT — see [`LICENSE`](LICENSE).

The Gig Performer SDK (vendored via `add_subdirectory`) retains its own license;
see the SDK repository’s `LICENSE.md`.
