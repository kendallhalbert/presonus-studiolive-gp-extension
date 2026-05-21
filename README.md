# PreSonus StudioLive — Gig Performer Extension

Native Windows extension (`PreSonusStudioLive.dll`) that connects
[Gig Performer](https://gigperformer.com) to a **PreSonus StudioLive III**
mixer over UCNet (TCP port 53000). Control the desk from **GPScript** using
`PreSonusStudioLive_*` functions — connect, LINE channel parameters, project/scene
lists, and scene recall.

**Independent third-party project** — not affiliated with, endorsed by, or sponsored
by PreSonus Audio Electronics, Inc. or Gig Performer.

This repo is the C++ implementation that supersedes the Node.js
[`@featherbear/presonus-studiolive-api`](https://github.com/featherbear/presonus-studiolive-api)
library. Wire-format behavior is validated against captured fixtures from a
**StudioLive 32R** (firmware 3.3.0.109659).

> **Status (2026-05-20):** Phase 0 and Phase 1 are **complete**. **Phase 2** is in
> progress: UCNet handshake, parameter cache, LINE GPScript APIs, and FD project/scene
> listing. **48 unit tests** across 24 executables. **Hardware verified** on a 32R:
> TCP connect and LINE mute set/get. Other LINE APIs and scene recall are implemented
> but not yet re-checked on hardware.
>
> Design, roadmap, and phase detail:
> [`docs/GP_EXTENSION_PLAN.md`](docs/GP_EXTENSION_PLAN.md).
> Mixer on-desk validation:
> [`docs/HARDWARE_SMOKE_TEST.md`](docs/HARDWARE_SMOKE_TEST.md).

---

## What works today

| Area | GPScript surface | Notes |
| ---- | ---------------- | ----- |
| Session | `Connect`, `Disconnect`, `IsConnected` | TCP to mixer IP/hostname, port 53000 |
| Logging | `SetLogLevel`, `LogFilePath` | File log under `%APPDATA%\PreSonusStudioLive\extension.log` |
| LINE inputs | Mute, level (0–100%), solo, pan, color | 1-based channel index; getters read optimistic cache |
| Projects / scenes | `GetProjectCount`, `GetProjectName`, `GetSceneCount`, `GetSceneName`, `RecallProjectScene` | First list call can block ~5s while FD payloads arrive |
| Meta | `Version` | Currently reports `1.0.0-phase0` |

**Planned (not in this build):** AUX / FX / MAIN / DCA / SUB paths, dB faders, fades,
GP rackspace widgets, and broader mix-type coverage — see the plan doc §4.

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

GPScript requires functions that return a value to appear inside `Print(...)` or an
assignment. `Disconnect()` may be called as a standalone statement.

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

Confirmed on **StudioLive 32R** @ `10.0.0.14`: connect and LINE 1 mute toggles the desk.

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
