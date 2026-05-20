# Gig Performer Extension — Project Plan

Port of the `@featherbear/presonus-studiolive-api` Node.js library to a native
Gig Performer extension (Windows DLL) that exposes the PreSonus StudioLive III
mixer control surface to **GPScript** via `psl_*` functions.

This document is the single source of truth for the design and roadmap. When
the new repo (`presonus-studiolive-gp-extension`) is scaffolded, this plan
will be copied/symlinked there so it sits next to the implementation.

---

## Changelog

| Date       | Change                                                                   |
| ---------- | ------------------------------------------------------------------------ |
| 2026-05-12 | Initial plan captured from Cursor design discussion.                     |
| 2026-05-12 | Phase 0 prep: capture script + runbook landed in JS repo. Paused awaiting VS install and the mixer capture session. |
| 2026-05-13 | User installed **Visual Studio 2026 Community** (v18.6, not VS 2022 as originally locked in) with the "Desktop development with C++" workload. Bundled CMake 4.2.3 + Ninja confirmed available. Revised locked-in decisions accordingly. |
| 2026-05-13 | Phase 0 delivered: sibling repo `presonus-studiolive-gp-extension` scaffolded, empty-DLL `PreSonusStudioLive.dll` built (configures, Debug-builds, sanity test green). Mixer-side load smoke test still owed by user. |
| 2026-05-13 | **Bookmarked / paused by user.** New C++ repo is `git init`'d but uncommitted; JS-repo changes also uncommitted. See "Repo state at bookmark" below for the exact `git status` snapshot. |
| 2026-05-18 | Phase 0 scaffold committed and pushed to `kendallhalbert/presonus-studiolive-gp-extension` on GitHub. CI workflow active. |
| 2026-05-18 | Fixtures captured against **StudioLive 32R** (firmware **3.3.0.109659**, serial RA3E18090022) and committed to `tests/fixtures/` in the C++ repo. Four optional fixtures skipped (see below). |
| 2026-05-18 | **Bookmarked / paused by user.** Capture session verified on-console (fader moved on mixer). GP smoke test and Phase 1 not started. See "Repo state at bookmark" below. |
| 2026-05-20 | **Bookmarked / paused by user.** GP 5 smoke test in progress — Release DLL + SDK v62 + product XML fixes landed locally (uncommitted). See "GP smoke test notes" below. |
| 2026-05-20 | Phase 0 GP smoke test **confirmed** on **Gig Performer 5 Pro** — `PreSonusStudioLive_Version()` prints `1.0.0-phase0`. Integration fixes still local/uncommitted. |
| 2026-05-20 | GP 5 integration fixes committed (`84219c0`). CI clones `beta-sdk-v62`. GP-bridge scaffold: Logger, Dispatcher, GpHost/RealGpHost/MockGpHost + unit tests. |
| 2026-05-20 | GP-bridge expanded: ExtensionContext drain at GP entry points, ScriptFunctions, ConfigStore. Phase 1 started: MessageProtocol + PvParser with fixture tests (10 tests). |
| 2026-05-20 | Phase 1: PC/PS/MS parsers, PacketParser dispatch, DataClient TCP deframer; 15 unit tests green. |
| 2026-05-20 | Phase 1: ZB/CK zlib + UBJSON + ZlibState (`CkAssembler`, `ZbParser`, zlib FetchContent); **17 tests** green. |

---

## Current status (resumption bookmark)

**Last updated: 2026-05-20 (Phase 1 ZB/CK + UBJSON)**

### TL;DR

Phase 0 **complete** on GP 5 Pro. GP-bridge done for now. Phase 1 progress:
`MessageProtocol`, PV/PC/PS/MS parsers, `DataClient` deframer, `parseWirePacket`,
ZB/CK inflate path (`CkAssembler`, `Ubjson`, `ZlibState`, `ZbParser`) —
**17 tests** green. **Next:** wire CK/ZB into session decoder / `PacketParser`,
TCP client + KeepAlive, FD/JM handlers.

### What's done

| Item                                                                 | Where                                                       |
| -------------------------------------------------------------------- | ----------------------------------------------------------- |
| This plan document                                                   | `docs/GP_EXTENSION_PLAN.md` in **both** the JS repo and the new C++ repo (kept in sync manually) |
| Fixture-capture script (~430 LOC, type-checks clean, dry-run verified) | `tools/capture-fixtures.ts` (JS repo)                     |
| Capture-script TypeScript check config                               | `tools/tsconfig.check.json` (JS repo)                       |
| Capture-script output ignore                                         | `tools/.gitignore` (`out/`) (JS repo)                       |
| Capture-session runbook (pre-flight checklist, expected output, troubleshooting) | `docs/CAPTURE_SESSION_RUNBOOK.md` (JS repo)    |
| npm scripts: `capture`, `capture:dry`, `capture:check`               | `package.json` (JS repo)                                    |
| Fixed `pnpm-workspace.yaml` (`dtrace-provider` build silenced for Windows) | `pnpm-workspace.yaml` (JS repo)                       |
| Visual Studio 2026 Community + "Desktop development with C++" workload installed (CMake 4.2.3 + Ninja bundled) | Local: `C:\Program Files\Microsoft Visual Studio\18\Community` |
| **Phase 0 scaffold + empty DLL** | `presonus-studiolive-gp-extension` (GoogleTest green; **GP 5 smoke test confirmed 2026-05-20**) |
| **Phase 0 GP acceptance** | GP 5 Pro — `PreSonusStudioLive_Version()` → `1.0.0-phase0` (Release DLL, SDK v62, `Name="PreSonus StudioLive"`) |
| **GitHub remote + CI** | `kendallhalbert/presonus-studiolive-gp-extension` (`main` @ `cc639d4` on remote; local edits ahead) |
| **Wire-level fixtures** | `tests/fixtures/` — StudioLive 32R, fw 3.3.0.109659 |
| **GP-bridge (partial)** | `extension/src/bridge/` — Logger, Dispatcher, GpHost, ExtensionContext, ScriptFunctions, ConfigStore |
| **Phase 1 (partial)** | `extension/src/protocol/` — MessageProtocol, PV/PC/PS/MS, DataClient, PacketParser, ZB/CK/UBJSON/ZlibState; 17 tests |
| **CI SDK v62** | `.github/workflows/ci.yml` clones `beta-sdk-v62` |

### GP smoke test notes (2026-05-20, confirmed)

User platform: **Gig Performer 5 Pro** (not 4.8). Extensions folder (GP 4.8+
docs): `C:\Users\Public\Documents\Gig Performer\Extensions\`.

Issues hit and fixes (committed in `84219c0`):

| Symptom | Cause | Fix |
| ------- | ----- | --- |
| Extension not listed / not loading | Wrong folder in original docs (`All Users`); Debug DLL depends on `*D.dll` runtimes | Install **Release** build to `Public\Documents\...\Extensions\` |
| "Extension too old" dialog | GP 5 requires SDK **v62+**; our build used SDK v47 | Check out `gp-sdk` branch **`beta-sdk-v62`** (`GPSDK_VERSION 62`) and rebuild |
| Unknown identifier `psl_Version` | Product XML used lowercase `name=` (ignored by GP 5); prefix fell back to DLL basename | Use **`Name=`** (capital N) in `<Product>` |
| Extension has no recognized name in UI | `Name="psl"` too terse / not human-readable | Use **`Name="PreSonus StudioLive"`** + `<Description>` + empty `<ImagePath>` |

**GPScript prefix rule (verified against gp-hud):** GP strips spaces/special
chars from Product `Name`. `Name="GP HUD"` → `GPHUD_*`. Ours:
`Name="PreSonus StudioLive"` → **`PreSonusStudioLive_Version()`** (not
`psl_Version()`). The original plan's `psl_` shorthand does not match GP's
prefix mechanism; revisit if we want a shorter prefix later.

**Release build for GP install** (verified on dev box 2026-05-20):

```powershell
# gp-sdk must be on beta-sdk-v62 before configuring
cd C:\Users\KenHa\source\repos\gigperformer\gp-sdk
git checkout beta-sdk-v62

cd C:\Users\KenHa\source\repos\presonus\presonus-studiolive-gp-extension
cmake -S . -B build-rel -G "Visual Studio 17 2022" -A x64
cmake --build build-rel --config Release --parallel
# DLL: build-rel\bin\Release\PreSonusStudioLive.dll (~42 KB Release vs ~905 KB Debug)
```

After copying DLL: **Options → Reload Third Party Libraries** (or restart GP).
Enable extension under **Options → Extensions**. Expect list row **PreSonus
StudioLive**, version `1.0.0-phase0`.

**Smoke test confirmed** with Release + SDK v62 + `Name=` product XML on GP 5 Pro:
`PreSonusStudioLive_Version()` prints `1.0.0-phase0` in the GP log.

### Fixture capture notes (2026-05-18)

Captured with `npm run capture -- --host <ip> --channel 1 --channel-type LINE`
(JS repo; use `npm` not `pnpm` — Node v16 on dev box, pnpm 11 not usable).
Host was passed explicitly, so discovery broadcast was not captured. ZB arrived
chunked (`03-handshake-zb-chunked/`). User **confirmed fader movement on the
mixer surface** during capture.

The following fixtures were **skipped** (non-fatal; re-run or hand-craft later
if needed):

| Skipped fixture | Reason |
| --------------- | ------ |
| `01-discovery-broadcast` | `--host` passed; discovery step not run |
| `04-pv-float-volume` | 32R does not echo volume as **incoming** `PV`; fader changes appear as **outgoing** `PV` + incoming **`MS`** frames. Use `13-ms-fader-sweep/` as the volume/fader oracle instead. |
| `06-pv-aux-send-level` | Channel may not be routable to AUX 1 |
| `18-meter-levl-frame` | Meter UDP frame not received in time |

All other required fixtures present, including chunked ZB handshake, PV/PC/PS
variants, MS fader sweep (7 frames), FD project/scene/preset lists, keepalive
probe, `snapshot-state.json`, and `session.jsonl`.

### Repo state at bookmark

**C++ repo** (`presonus-studiolive-gp-extension`) — **local commits ahead of
origin** (push when ready):

```
Branch:  main (tracks origin/main)
Remote:  https://github.com/kendallhalbert/presonus-studiolive-gp-extension.git
Ahead:   GP 5 integration commit + GP-bridge scaffold (pending push)
```

**GP install DLL** (local Release, SDK v62):

```
C:\Users\KenHa\source\repos\presonus\presonus-studiolive-gp-extension\build-rel\bin\Release\PreSonusStudioLive.dll
```

Legacy Debug DLL (do **not** install in GP — missing `MSVCP140D.dll` on non-dev PCs):

```
...\build\bin\Debug\PreSonusStudioLive.dll
```

**GP SDK clone** (sibling, not a submodule):

```
C:\Users\KenHa\source\repos\gigperformer\gp-sdk
Branch: beta-sdk-v62  (GPSDK_VERSION 62 — required for GP 5)
```

**JS repo** — unchanged from prior bookmark; **do not push**.

### Tooling notes at bookmark

| Tool | Status |
| ---- | ------ |
| Visual Studio 2026 + CMake 4.2.3 + Ninja | Installed |
| GitHub CLI (`gh`) v2.92.0 | Installed; authenticated as `kendallhalbert` |
| Node.js | v16.14.0 on PATH — **too old for pnpm 11**; use `npm run capture*` in JS repo |
| GP SDK clone | `gp-sdk` on branch **`beta-sdk-v62`** for GP 5 builds (main @ v47 is too old for user's GP 5) |
| Git author | Not configured globally; commits use transient `-c user.name/email` unless user sets config |

### What's blocked

| Blocker | Owner | Notes |
| ------- | ----- | ----- |
| GP-bridge remainder | Agent | Widget binding registry, feedback-loop suppression (Phase 3) |
| Phase 1 protocol port | Agent | ZB/CK + Ubjson, ZlibState, TCP client, KeepAlive, FD/JM handlers |
| Optional fixture re-capture (`01`, `06`, `18`) | User | Low priority |

### Phase 0 GP-side smoke test (updated 2026-05-20)

1. Ensure `gp-sdk` is on **`beta-sdk-v62`**, then build **Release**:

   ```powershell
   cd C:\Users\KenHa\source\repos\presonus\presonus-studiolive-gp-extension
   cmake -S . -B build-rel -G "Visual Studio 17 2022" -A x64
   cmake --build build-rel --config Release --parallel
   ```

2. Copy **`build-rel\bin\Release\PreSonusStudioLive.dll`** to
   `C:\Users\Public\Documents\Gig Performer\Extensions\`.

3. Restart GP (or **Options → Reload Third Party Libraries**). Open
   **Options → Extensions** — expect **PreSonus StudioLive** with version
   `1.0.0-phase0`. Enable it.

4. Rackspace GPScript:

   ```
   Initialization
       Print(PreSonusStudioLive_Version())
   End
   ```

5. Save rackspace. GP log should print `1.0.0-phase0`.

If `PreSonusStudioLive_Version` is still unknown: confirm Release DLL date,
extension enabled, script editor reopened after reload, and Product XML uses
`Name=` not `name=`.

### Resume path (priority order)

1. ~~**User**: install Visual Studio Community with the "Desktop development with C++" workload.~~ **Done 2026-05-13** — VS 2026 (v18.6) installed with bundled CMake 4.2.3 + Ninja.
2. ~~**Agent**: scaffold the sibling repo + Phase 0 DLL.~~ **Done 2026-05-13** — see new repo at `C:\Users\KenHa\source\repos\presonus\presonus-studiolive-gp-extension`.
3. ~~**User → Agent**: explicitly authorise the initial commit of the new C++ repo.~~ **Done 2026-05-18** — `9b85664 Initial Phase 0 scaffold`, pushed to GitHub.
4. ~~**User**: GP-side smoke test.~~ **Done 2026-05-20** — `PreSonusStudioLive_Version()` confirmed on GP 5 Pro.
5. ~~**User**: mixer capture session.~~ **Done 2026-05-18**.
6. ~~**Agent**: copy fixtures.~~ **Done 2026-05-18**.
7. ~~**Agent**: commit GP smoke-test fixes.~~ **Done 2026-05-20** (`84219c0`).
8. ~~**Agent**: GP-bridge infrastructure.~~ **Mostly done 2026-05-20** — drain at GP entry points (no SDK timer); ScriptFunctions + ConfigStore landed.
9. **Agent**: Phase 1 protocol port — **in progress** (framing + PV/PC/PS/MS parsers + DataClient; ZB/UBJSON next).
10. Continue Phases 2–5 per §5.

### How to reproduce the verified build from scratch

```powershell
# from any PowerShell, the dev shell is loaded by the script itself:
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Launch-VsDevShell.ps1' -Arch amd64 -SkipAutomaticLocation
cd C:\Users\KenHa\source\repos\presonus\presonus-studiolive-gp-extension
cmake -S . -B build -G "Ninja Multi-Config" -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl
cmake --build build --config Debug --parallel
ctest --test-dir build --build-config Debug --output-on-failure
```

Expected: configure ~28 s, build 22 targets clean on `/W4 /permissive-`,
`2/2` tests pass, DLL at `build\bin\Debug\PreSonusStudioLive.dll`
(~880 KB).

### Useful entry points when resuming

- §2 — locked-in decisions table (the contract for everything that follows)
- §4 — full `psl_*` GPScript surface
- §5 — phased roadmap
- §6 — testing strategy
- §7 — fixture inventory and capture sequence
- `docs/CAPTURE_SESSION_RUNBOOK.md` (JS repo) — what to actually do during the mixer session
- `presonus-studiolive-gp-extension/README.md` — local-build + smoke-test instructions for the new repo

### Things floated but not started (low priority, both Phase-1 quality-of-life)

- `tools/inspect-fixture.ts` — pretty-print a captured `.bin` file (header / message code / decoded payload). Useful for debugging C++ parser discrepancies. Recommended before Phase 1 starts.
- `docs/PROTOCOL_NOTES.md` — distill what the JS code knows about UCNet (key naming conventions, PV/PC/PS/ZB/CK/FD/MS layouts, the `assign_aux` inversion gotcha, the `aux<odd><even>_stpan` pan key pattern, etc.) into one human-readable reference. Avoids spelunking through 30 `.ts` files when porting to C++.

### Original Cursor design discussion

The full Q&A that produced this plan is in the Cursor agent transcript folder for this project. Search for transcripts dated 2026-05-12 to replay the architectural alternatives considered (sidecar Node process, embedded libnode) and the rationale for the native C++ path.

---

## 1. Goal & non-goals

### Goal
Allow Gig Performer 4.8+ users to control a PreSonus StudioLive III mixer
(LINE/AUX/FX/MAIN/DCA/SUB faders, mutes, solos, scenes, etc.) directly from
GPScript, with first-class **two-way widget mirroring** so on-screen widgets
can both reflect and drive mixer state during a live performance.

The C++ extension becomes the canonical implementation going forward; the
existing TypeScript library is retired after fixture capture is complete.

### Non-goals (for v1)
- macOS / Linux support (Windows-only by user choice; revisit later).
- Multiple simultaneous mixer connections.
- Authentication / encryption layers (private-network deployment assumed).
- Signed DLL distribution (the user accepts the SmartScreen prompt).
- Replacement for the StudioLive Remote / UC-Surface app UI; we are only the
  control plane.
- MIDI translation layer (the GP user can already write GPScript that converts
  `psl_*` calls into MIDI on their own if desired).

---

## 2. Locked-in decisions

| Decision                       | Choice                                                                                                          |
| ------------------------------ | --------------------------------------------------------------------------------------------------------------- |
| Target repo                    | `C:\Users\KenHa\source\repos\presonus\presonus-studiolive-gp-extension` (sibling of the JS repo)                |
| Platform                       | Windows x64 only                                                                                                |
| Min Gig Performer version      | **4.8+** design target; **user runs GP 5 Pro** — requires SDK **v62** (`beta-sdk-v62` branch), not main @ v47 |
| Language / standard            | C++20, MSVC from Visual Studio 2026 (v18.6) — VS 2022 (v17) used for `build-rel` Release on dev box |
| Build system                   | CMake ≥ 4.2. Default generator: **"Visual Studio 18 2026"**; GP Release verified with **"Visual Studio 17 2022"** `-A x64`. CI uses **"Ninja Multi-Config"**. |
| Output                         | `PreSonusStudioLive.dll` **Release** build → `C:\Users\Public\Documents\Gig Performer\Extensions\` (never install Debug in GP) |
| GPScript prefix                | Derived from Product **`Name=`** (capital N). Current: `PreSonusStudioLive_` from `Name="PreSonus StudioLive"`. Plan API names still say `psl_*` — rename in Phase 2 registration or accept long prefix. |
| GP SDK reference               | `add_subdirectory(${GP_SDK_DIR})`; default `C:/Users/KenHa/source/repos/gigperformer/gp-sdk`. **For GP 5: checkout `beta-sdk-v62` locally** before building. CI clones `beta-sdk-v62`. |
| Third-party deps               | CMake `FetchContent` for zlib and GoogleTest. No vcpkg.                                                         |
| Concurrency model              | Single mixer connection; one IO worker thread + one GP-thread dispatcher queue                                  |
| Connection lifecycle           | Auto-discover + auto-connect to first found mixer on `OnOpen`; `psl_Connect` / `psl_Disconnect` override        |
| Widget binding direction       | Per-binding `direction` argument: `0 = mixer→widget`, `1 = widget→mixer`, `2 = both`                            |
| Widget level scale             | Two families: `psl_BindLevelWidgetLinear` (0..1 ↔ 0..100% WYSIWYG) and `psl_BindLevelWidgetDb` (0..1 ↔ -84..+10 dB) |
| Song ↔ scene auto-recall       | Phase 3 — `psl_BindSongToScene`, in-memory bindings, user re-applies from GPScript on each gig load             |
| Source of truth                | C++ extension is canonical; JS library retired after fixture capture                                            |
| Trust model                    | Private-network deployment; no auth/TLS, plaintext config, no DPAPI                                             |

---

## 3. Architecture overview

```
                    Gig Performer process
   +-------------------------------------------------+
   |  GP UI / audio thread                           |
   |   |- GPRegister, OnOpen, OnClose, OnSong*       |
   |   |- psl_* GPScript entrypoints (stack ABI)     |
   |   '- DispatcherDrain (timer-fed task pump)      |
   |                  ^ post tasks                   |
   |                  |                              |
   |  IO worker thread (owned by extension)          |
   |   |- TCP client -> packet framer -> handlers    |
   |   |- KeepAlive (1s tick, 3s timeout)            |
   |   |- ZB state tree + KV cache (mutex-protected) |
   |   '- UDP meter listener (Phase 5)               |
   |                  ^                              |
   +------------------|------------------------------+
                      | TCP :53000 / UDP :47809
                      v
                StudioLive console
```

### Key abstractions for testability

- **`Transport` interface** — production: `TcpTransport`; tests:
  `FakeMixerTransport` or `LoopbackTransport`. `Client` never touches a socket
  directly.
- **`GpHost` interface** — wraps every `GP_*` function pointer call. Production:
  `RealGpHost`; tests: `MockGpHost` that records calls and supplies canned
  responses.
- **`Dispatcher`** — lock-free single-producer/single-consumer queue, drained
  on the GP thread. Tests run synchronously by draining the queue manually.

### Concurrency rules

- All `GP_*` calls happen on the GP thread.
- All socket I/O happens on the IO thread.
- The KV state cache is read by both; protected by one `std::mutex`.
- Mixer→widget updates: IO thread pushes a task onto the dispatcher; GP-thread
  drain calls `GpHost::setWidgetValue`.
- Widget→mixer updates: GP-thread `OnWidgetValueChanged` enqueues a packet
  send via the IO thread's command queue.
- **Feedback-loop suppression**: when we push a value to a widget because of
  a mixer update, record `{widget → expected value, expiresAt}`; the next
  matching `OnWidgetValueChanged` is swallowed if it arrives within ~50 ms.
  Symmetric behaviour for widget→mixer.

---

## 4. GPScript surface (v1)

GPScript supports only `Integer`, `Double`, `Boolean`, and `String`. The
`ChannelSelector` object from the JS API is therefore flattened into four
parameters: `type`, `channel`, `mixType`, `mixNumber`. For non-send operations
pass `mixType=""` and `mixNumber=0`.

Channel `type` values: `"LINE" | "RETURN" | "FXRETURN" | "TALKBACK" | "AUX" | "FX" | "SUB" | "MAIN" | "DCA"` (plus `"MASTER"` / `"MONO"` on 64S models).

Widget `direction` values: `0 = mixer→widget`, `1 = widget→mixer`, `2 = both`.

### 4.1 Connection & discovery

```
psl_Version()           Returns String
psl_IsConnected()       Returns Boolean
psl_Connect(host : String, port : Integer)            Returns Boolean
psl_Disconnect()
psl_GetConnectedHost()  Returns String
psl_GetConnectedName()  Returns String

psl_Discover(timeoutMs : Integer)                     Returns Integer
psl_GetDiscoveredHost(index : Integer)                Returns String
psl_GetDiscoveredName(index : Integer)                Returns String
psl_GetDiscoveredSerial(index : Integer)              Returns String
psl_DiscoverAndConnect(timeoutMs : Integer)           Returns Boolean
```

### 4.2 Channel introspection

```
psl_GetChannelCount(type : String)                    Returns Integer
```

### 4.3 Mute / Solo

```
psl_SetMute   (type : String, channel : Integer, mixType : String, mixNumber : Integer, muted : Boolean)
psl_GetMute   (type : String, channel : Integer, mixType : String, mixNumber : Integer)  Returns Boolean
psl_ToggleMute(type : String, channel : Integer, mixType : String, mixNumber : Integer)

psl_SetSolo   (type : String, channel : Integer, soloed : Boolean)
psl_GetSolo   (type : String, channel : Integer)  Returns Boolean
psl_ToggleSolo(type : String, channel : Integer)
```

### 4.4 Levels

```
psl_SetLevelLinear(type, channel, mixType, mixNumber, level : Double, fadeMs : Integer)   // level 0..100
psl_SetLevelDb    (type, channel, mixType, mixNumber, db    : Double, fadeMs : Integer)   // -84..+10
psl_GetLevelLinear(type, channel, mixType, mixNumber)  Returns Double
psl_GetLevelDb    (type, channel, mixType, mixNumber)  Returns Double
```

### 4.5 Pan / Color

```
psl_SetPan  (type : String, channel : Integer, pan : Double)            // 0..100
psl_SetColor(type : String, channel : Integer, rgbHex : String, alpha : Integer)
psl_GetColor(type : String, channel : Integer)                          Returns String
```

### 4.6 Scenes / Projects

```
psl_GetProjectCount()                                   Returns Integer
psl_GetProjectName(index : Integer)                     Returns String
psl_GetSceneCount(projFile : String)                    Returns Integer
psl_GetSceneName (projFile : String, index : Integer)   Returns String
psl_GetCurrentProject()                                 Returns String
psl_GetCurrentScene()                                   Returns String

psl_RecallProject     (projFile : String)
psl_RecallProjectScene(projFile : String, sceneFile : String)
psl_RecallChannelStrip(type : String, channel : Integer, chanFile : String)
```

### 4.7 Widget mirroring (Phase 3)

```
psl_BindLevelWidgetLinear(widget : String, type, channel, mixType, mixNumber, direction : Integer)  Returns Boolean
psl_BindLevelWidgetDb    (widget : String, type, channel, mixType, mixNumber, direction : Integer)  Returns Boolean
psl_BindMuteWidget       (widget : String, type, channel, mixType, mixNumber, direction : Integer)  Returns Boolean
psl_BindSoloWidget       (widget : String, type, channel, direction : Integer)                      Returns Boolean
psl_UnbindWidget         (widget : String)                                                          Returns Boolean
psl_UnbindAll()
```

### 4.8 Song ↔ scene auto-recall (Phase 3)

```
psl_BindSongToScene    (songIndex : Integer, projFile : String, sceneFile : String)                 Returns Boolean
psl_BindSongPartToScene(songIndex : Integer, partIndex : Integer, projFile : String, sceneFile : String)  Returns Boolean
psl_UnbindSong(songIndex : Integer)                                                                  Returns Boolean
psl_ClearSongBindings()
```

### 4.9 Misc

```
psl_Panic()                                  // unmute all + master to 72%
psl_SetLogLevel(level : String)              // "info","debug","warn","error","none"
```

---

## 5. Phased delivery roadmap

### Phase 0 — Scaffolding (~1 day)

**Deliverable**: An empty DLL that GP can load and call.

- CMake project, MSVC 2022 x64 build, C++20.
- `add_subdirectory(${GP_SDK_DIR})` (default = absolute path to local SDK clone).
- `entry.cpp` provided by the SDK is linked in unchanged.
- Implement `CreateGPExtension` and a `LibMain : GigPerformerAPI` subclass.
- One GPScript function: `psl_Version() Returns String` returns `"1.0.0-phase0"`.
- CMake `install` target copies the DLL to
  `C:\Users\Public\Documents\Gig Performer\Extensions\`.
- Verify in GP: extension appears in extensions list, can be enabled, the
  GPScript function is callable.

### Phase 1 — Protocol core (~1 week)

**Deliverable**: All wire-level decoders/encoders, tested against captured
fixtures.

- Port `messageProtocol`, `bufferUtil`, packet framing (`DataClient`),
  `UniqueRandom`, `KVTree`, `CacheProvider`, `treeUtil`.
- Port `ubjson.ts` → `Ubjson.cpp`.
- Port `zlibNodeParser` + `zlibUtil` → `ZlibState.cpp` (uses `FetchContent` zlib).
- TCP client + KeepAlive on the IO worker thread.
- Packet handlers: `PV`, `PS`, `PC`, `ZB`, `CK`, `FD`, `MS`, `JM`.
- Channel-string parsing (port of `channelUtil.ts`).
- `valueUtil`: `logVolumeToLinear`, `transitionValue`.
- All under unit tests with fixtures from the capture session (§7).

### Phase 2 — Core GPScript surface (~1 week)

**Deliverable**: Mute / level / solo / color / pan / scene-recall callable
from GPScript end-to-end against a real mixer.

- GPScript registration table.
- All `psl_*` functions from sections 4.1–4.6.
- Stack-based ABI helpers (`PopChannelSelector`, etc.) wrapping `GP_VM_*`.
- Input validation library (rejects bad type strings, out-of-range channels;
  every `psl_*` returns a sentinel rather than crashing on bad input).
- Fade transitions (`transitionValue` on the IO thread).
- Per-function tests via `MockGpHost` + `FakeMixerTransport`.

### Phase 3 — Events, widgets, song bindings (~3–5 days)

**Deliverable**: Live widget mirroring + GP-song-driven scene recall.

- Dispatcher queue + GP-thread drain (timer-driven via a recurring callback).
- Widget binding registry + feedback-loop suppression.
- `psl_BindLevelWidgetLinear` / `Db`, `psl_BindMuteWidget`, `psl_BindSoloWidget`,
  `psl_UnbindWidget`, `psl_UnbindAll`.
- `OnSongChanged` / `OnSongPartChanged` callbacks → consult song-binding table
  → call `recallProjectScene` on the IO thread.
- Demo gig file (`gigs/Demo.gig`) with mirrored fader and mute widgets.

### Phase 4 — Discovery + presets (~3 days)

**Deliverable**: Auto-connect on `OnOpen`; project / scene / channel-preset
enumeration callable from GPScript.

- UDP discovery (port 47809 broadcast listener).
- `sendList` with `FD` chunk reassembly: projects, scenes-of-project, channel
  presets.
- `psl_Discover` / `psl_DiscoverAndConnect`, `psl_GetProjectCount` / `Name`, etc.
- Config persistence in `%APPDATA%\PreSonusStudioLive\config.json`
  (last-known host + serial; used to prefer the previous mixer on next start).

### Phase 5 — Metering (optional, ~3 days)

**Deliverable**: Real-time meter levels accessible from GPScript.

- UDP meter server, `levl` frame parsing.
- `psl_SubscribeMeters()` / `psl_UnsubscribeMeters()`.
- `psl_GetMeterLevel(groupId : Integer, channel : Integer)` returns the most
  recent value (cached; non-blocking).
- Optional: `psl_BindMeterWidget(widget, groupId, channel)`.

---

## 6. Testing strategy

### 6.1 Tooling

- **Framework**: GoogleTest 1.14+ via `FetchContent`. GoogleMock for the
  `GpHost` and `Transport` mocks.
- **Runner**: CTest. One test executable per file for isolation
  (`test_ubjson.exe`, `test_zlibstate.exe`, …).
- **Coverage**: OpenCppCoverage, emits Cobertura + HTML. `coverage` CMake target.
- **Sanitizers**: MSVC `/fsanitize=address` for an ASan build job in CI.
  (UBSan unavailable on MSVC; clang-tidy `bugprone-*` and `cert-*` checks
  fill some of the gap.)
- **Static analysis**: MSVC `/analyze` in CI, clang-tidy on `extension/src/`.
- **CI**: GitHub Actions, Windows runners. Jobs: Debug + tests, Release + tests,
  Debug + ASan + tests. Coverage report uploaded as artifact.
- **Branch protection**: CI must be green before merge to `main`.

### 6.2 Testability tiers

| Tier | Examples | Strategy |
| --- | --- | --- |
| Pure logic | UBJSON, MessageProtocol, ChannelUtil, ValueUtil, KVTree, Transformers | Standard unit tests |
| Protocol-with-fixtures | ZlibState, PV/PS/PC/ZB/CK/FD/MS handlers | Compare against captured `.bin` / `.json` oracle pairs |
| Networked client | `Client`, `KeepAlive`, `Discovery`, reconnect, chunked FD | In-process `FakeMixer` over loopback |
| GP bridge | `psl_*` entrypoints, dispatcher, widget mirroring | `MockGpHost` + `FakeMixerTransport` |

### 6.3 Coverage targets

- Pure-logic and protocol layers: **≥ 90% line coverage**.
- `Client.cpp`: **≥ 75%**, with every named state transition exercised by at
  least one named test (connect→handshake, handshake→connected, connected→
  keepalive_timeout→reconnect, connected→close).
- Every `psl_*` function: **at least one happy-path test and one bad-input
  test**. Bad-input tests assert *"does not crash, returns a documented sentinel
  value"* — that is the GP-stability contract from the SDK README.

### 6.4 Adversarial fixtures

Hand-edited `.bin` files for malformed input (truncated headers, wrong-length
prefixes, invalid UBJSON control bytes, oversize chunks). Each one drives a
test that asserts *"parser rejects cleanly, no crash, no UB under ASan"*.

### 6.5 Explicit non-goals for automated testing

- GP's internal behaviour (widget update propagation, callback dispatching,
  thread semantics). Covered by the manual smoke checklist.
- Real-mixer interaction beyond fixture capture. Covered by the manual smoke
  checklist.
- DLL load/unload across multiple GP launches. The SDK's static `subclass`
  pattern is opaque to us.

### 6.6 Manual smoke checklist (pre-release)

Lives at `tests/manual-smoke.md` in the new repo.

1. Drop DLL in Extensions folder, start GP, verify metadata in the extensions UI.
2. Open known gig with `psl_*` calls in `On Open`, verify connection log.
3. Mute / unmute a known channel from GPScript → verify on mixer surface.
4. Move fader on mixer → verify bound widget moves.
5. Kill mixer power for 5 s → verify reconnect.
6. Switch GP song → verify scene recall.
7. Quit GP → verify clean disconnect (`Unsubscribe` packet sent).

---

## 7. Fixture capture plan

### 7.1 The capture script

A `tools/capture-fixtures.ts` script is added to the **existing JS repo**
(`presonus-studiolive-api`) and run **once** against the real mixer before the
JS lib is retired. Output goes to `tools/out/fixtures/`, which is then
committed to the new GP-extension repo under `tests/fixtures/`.

### 7.2 Fixture inventory

| File / folder                       | What it captures                                                    |
| ----------------------------------- | ------------------------------------------------------------------- |
| `01-discovery-broadcast.bin`        | UDP broadcast announcement from the mixer                           |
| `02-handshake-zb.bin`               | Initial ZB state snapshot (largest single fixture)                  |
| `03-handshake-zb-chunked.bin`       | When the ZB arrives split across `CK` frames                        |
| `04-pv-float-volume.bin`            | `line/ch1/volume = 0.72`                                            |
| `05-pv-bool-mute.bin`               | `line/ch1/mute = true`                                              |
| `06-pv-aux-send-level.bin`          | `line/ch1/AUX1 = 0.5`                                               |
| `07-pv-fx-send-level.bin`           | `line/ch1/FXA = 0.5`                                                |
| `08-pv-assign-aux.bin`              | `line/ch1/assign_aux1` mute-on-send                                 |
| `09-pv-link-stereo.bin`             | `line/ch1/link = true`                                              |
| `10-pv-pan.bin`                     | `line/ch1/pan = 0.5`                                                |
| `11-pc-color.bin`                   | `line/ch1/color = "FF8800"`                                         |
| `12-ps-channel-name.bin`            | `line/ch1/username = "Kick"`                                        |
| `13-ms-fader-sweep/`                | Sequence of `MS` frames captured during a 200 ms fade               |
| `14-fd-projects-list/`              | Chunk-by-chunk `FD` response for `presets/proj`                     |
| `15-fd-channel-presets/`            | Chunk-by-chunk `FD` response for `presets/channel`                  |
| `16-fd-scenes-of-project/`          | Chunk-by-chunk `FD` response for `presets/proj/<first project>`     |
| `17-keepalive-fr-response.bin`      | The `FD` probe used by `KeepAliveHelper`                            |
| `18-meter-levl-frame.bin`           | One UDP frame from `meterSubscribe`                                 |
| `snapshot-state.json`               | Full `client.dumpState()` after handshake                           |
| `snapshot-after-mute.json`          | State diff after mute toggle                                        |
| `snapshot-after-fade.json`          | State diff after the fade                                           |
| `session.log`                       | Human-readable transcript of what was captured                      |
| `99-malformed-*.bin`                | Hand-edited adversarial inputs                                      |

Each `.bin` is the raw bytes received on the wire (header included). Each
sidecar `.json` is the expected parsed output produced by the JS parsers —
the oracle that the C++ tests assert against.

### 7.3 Capture sequence (~30 s runtime)

1. UDP discovery listen for 5 s, capture broadcast.
2. Connect, capture ZB handshake (and CK frames if any).
3. `sendList` for projects / channel presets / scenes-of-first-project.
4. Snapshot full state.
5. Mute ch1 → capture PV. Unmute.
6. Set ch1 color → capture PC. Restore.
7. Rename ch1 username → capture PS. Restore.
8. Subscribe to meter data, capture one frame, unsubscribe.
9. Drive a 200 ms fade on a known channel, capture every MS frame.
10. Snapshot diffed state.
11. Disconnect cleanly.

Idempotent — safe to re-run if a capture comes out incomplete.

### 7.4 Pre-session checklist

- [ ] Mixer powered on, ethernet-connected to the same subnet as the dev PC.
- [ ] Mixer IP known (or rely on discovery).
- [ ] At least **one project** saved on the mixer with **one scene** in it
      (so the FD list fixtures aren't empty).
- [ ] At least **one channel preset** saved on the mixer.
- [ ] **One channel (e.g. Line ch1) is "safe to poke"** — the script briefly
      mutes / unmutes, recolors, renames, and fades it. Choose a channel with
      nothing connected if possible.
- [ ] Existing JS repo runs (`pnpm install` succeeded, `npm run run` connects
      with the hardcoded host).

---

## 8. Tooling defaults

- **Compiler / generator**: MSVC from VS 2026 (v18.6), x64. CMake's default
  "Visual Studio 18 2026" generator for local IDE-friendly builds; CI uses
  "Ninja Multi-Config" with the same toolchain (faster, deterministic). C++20.
- **GP SDK reference**: `add_subdirectory(${GP_SDK_DIR})`, default
  `C:/Users/KenHa/source/repos/gigperformer/gp-sdk`. Overridable via
  `-DGP_SDK_DIR=...`. No git submodule.
- **Third-party deps**: CMake `FetchContent` for zlib and GoogleTest.
  No vcpkg, no Conan.
- **Output artifact**: `PreSonusStudioLive.dll`.
- **Install target**: `cmake --install build --component dev` copies the DLL
  to `C:\Users\Public\Documents\Gig Performer\Extensions\`.
- **Code style**: `.clang-format` matching the GP SDK's style.
- **Naming**:
  - C++ types: `PascalCase`.
  - Free functions / methods: `camelCase`.
  - Constants: `kCamelCase` or `UPPER_SNAKE` (TBD; pick once and stick).
  - File names: `PascalCase.{h,cpp}` matching the primary type.
  - All extension code under `namespace presonus::studiolive::gpext`.

---

## 9. Open items / future decisions

These were noted but explicitly punted for v1. Revisit when relevant.

- **macOS support.** GP runs on macOS too; the protocol code is portable, the
  GP-bridge code uses cross-platform SDK APIs, the socket layer needs minor
  abstraction (`closesocket` vs `close`, `WSAStartup` etc.). Realistic but
  out of scope now.
- **DLL code signing.** Required for SmartScreen-free distribution to other
  users. Not needed for the original user's private deployment.
- **Persistent widget bindings across gigs.** Currently bindings are applied
  via GPScript `On Open` callbacks. Could persist by gig UUID in
  `%APPDATA%`; defer until proven friction.
- **MIDI bridge.** Translate mixer events into virtual MIDI messages via
  `GP_InjectMidiMessageToMidiInputAlias` so existing MIDI-only rackspaces can
  consume them. Considered and rejected for v1 (lossy 7-bit CC range, ugly).
- **Metering widget binding.** Listed as Phase 5 stretch in §5.
- **Plugin / VST parameter mirroring.** GP's `MapWidgetToPluginParameter`
  exists; could mirror a mixer parameter directly to a VST plugin parameter.
  Speculative.
- **Headless mode / standalone CLI.** Out of scope (the JS lib already plays
  this role for testing).
- **Telemetry / crash reporting.** Out of scope; logging to local file is
  sufficient on a private network.

---

## 10. Glossary

- **UCNet** — PreSonus's proprietary TCP/UDP protocol that the StudioLive III
  surface, the UC-Surface app, and StudioLive Remote all use.
- **UBJSON** — Universal Binary JSON. The serialisation format used inside
  the ZB payload. https://ubjson.org
- **ZB** — UCNet message code for the zlib-compressed UBJSON state snapshot
  sent on connect.
- **PV** — `ParamValue`. A typed key/value update for a leaf in the mixer
  state tree (e.g. `line/ch1/volume = 0.72`).
- **PS** — `ParamString`. String key/value update (e.g. channel name).
- **PC** — `ParamChars`. Raw-bytes key/value update (e.g. color = 4 bytes
  RGBA).
- **CK** — `Chunk`. A fragment of a larger payload that must be reassembled.
- **FD** — `FileData`. A response to a `FileRequest`; carries file-list or
  file-content data, often chunked.
- **MS** — `Meter16` / fader-position broadcast. Linear position of all
  faders and sends, packed as `uint16` per channel.
- **KA** — `KeepAlive`. Heartbeat probe.
- **ChannelSelector** — JS-side `{ type, channel, mixType?, mixNumber? }`
  object. Flattened to four GPScript args.
- **Mix send vs direct channel** — Without `mixType` you address the channel
  itself (volume / mute on the main bus). With `mixType` set to `AUX` or `FX`
  you address that channel's contribution to the specified send bus.
- **Send-on / send-off (`assign_*`)** — Whether a channel is included in a
  specific aux or fx send. Inverted semantics vs the channel's own mute.
- **Filtergroup** — UCNet's wire name for DCAs.

---

## 11. Reference

The design discussion that produced this plan lives in the Cursor agent
transcript folder for this project
(`C:\Users\KenHa\.cursor\projects\c-Users-KenHa-source-repos-presonus-presonus-studiolive-api\agent-transcripts\`).
Open the most recent transcript dated 2026-05-12 to replay the full Q&A,
including the architectural alternatives considered (sidecar Node process,
embedded libnode) and the rationale for the native C++ path.
