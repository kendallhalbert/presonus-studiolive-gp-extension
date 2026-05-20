# PreSonus StudioLive — Gig Performer Extension

Native Windows extension (`PreSonusStudioLive.dll`) that lets
[Gig Performer](https://gigperformer.com) 4.8+ control a **PreSonus
StudioLive III** mixer (LINE / AUX / FX / MAIN / DCA / SUB faders, mutes,
solos, scenes, etc.) directly from **GPScript** via `psl_*` functions.

This is a C++ port of the
[`@featherbear/presonus-studiolive-api`](https://github.com/featherbear/presonus-studiolive-api)
Node.js library. The C++ extension becomes the canonical implementation
going forward; the JS library is retired once fixture capture is complete.

> **Status**: Phase 0 — scaffolding. The DLL builds, loads, and exposes
> `psl_Version()` from GPScript. No mixer protocol yet. See
> [`docs/GP_EXTENSION_PLAN.md`](docs/GP_EXTENSION_PLAN.md) for the full
> roadmap.
>
> The GPScript prefix comes from the extension product XML `Name` attribute
> (capital N — see `LibMain.cpp::GetProductDescription`). GP strips spaces from
> the name, so `Name="PreSonus StudioLive"` yields `PreSonusStudioLive_Version()`.
> The `Description` field is the longer blurb shown alongside the name.

---

## Prerequisites

| Tool                                         | Tested version             |
| -------------------------------------------- | -------------------------- |
| Visual Studio 2026 Community / Professional  | 18.6.x (Native Desktop)    |
| CMake                                        | 4.2.3 (bundled with VS)    |
| Ninja (optional, but recommended for CI)     | bundled with VS            |
| Gig Performer                                | ≥ 4.8 (SDK v47)            |
| Gig Performer SDK clone                      | latest `main` at `https://github.com/gigperformer/gp-sdk` |

The SDK clone is expected at
`C:/Users/KenHa/source/repos/gigperformer/gp-sdk` by default. Override with
`-DGP_SDK_DIR=<your/path>`.

---

## Build

The simplest path (uses VS 2026's bundled CMake + Ninja Multi-Config):

```powershell
# from a "Developer PowerShell for VS 2026" prompt
cmake -S . -B build -G "Ninja Multi-Config"
cmake --build build --config Debug
ctest --test-dir build --build-config Debug --output-on-failure
```

Or via the IDE-friendly Visual Studio generator:

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64
cmake --build build --config Debug
ctest --test-dir build --build-config Debug --output-on-failure
```

Either way, the DLL ends up at:

```
build/bin/Debug/PreSonusStudioLive.dll      # local dev/tests only — do NOT install in GP
build-rel/bin/Release/PreSonusStudioLive.dll # use this for Gig Performer (see below)
```

> **Important:** Gig Performer cannot load a **Debug** build. Debug DLLs depend on
> `MSVCP140D.dll` / `VCRUNTIME140D.dll` / `ucrtbased.dll`, which are only present
> when Visual Studio is installed. GP will silently ignore the extension. Always
> install a **Release** build into the Extensions folder.

Release build (VS 2022 generator example):

```powershell
cmake -S . -B build-rel -G "Visual Studio 17 2022" -A x64
cmake --build build-rel --config Release --parallel
```

To install into Gig Performer's Extensions folder:

```powershell
cmake --install build-rel --config Release --component dev
```

(requires write access to `C:\Users\Public\Documents\Gig Performer\Extensions\`)

---

## Manual smoke test (Phase 0 acceptance)

1. Build **Release** as above (not Debug).
2. Copy `build-rel/bin/Release/PreSonusStudioLive.dll` into
   `C:\Users\Public\Documents\Gig Performer\Extensions\`.
3. Start Gig Performer.
4. Open **Options → Extensions** — **PreSonus StudioLive** should appear
   in the list with version `1.0.0-phase0`. Tick the box to enable it.
   Restart GP if prompted.
5. Create a scratch rackspace, open the GPScript window for the rackspace,
   and add:

    ```gigperformer
    Initialization
        Print(PreSonusStudioLive_Version())
    End
    ```

6. Save the rackspace. The GP log should print
   `1.0.0-phase0`.

Steps 1–6 are the Phase 0 acceptance criteria (GP load + GPScript version call).
Confirmed on Gig Performer 5 Pro, 2026-05-20.

---

## Repository layout

```
.
├── CMakeLists.txt              # top-level project, picks up GP SDK + tests
├── extension/                  # the DLL itself
│   ├── CMakeLists.txt
│   └── src/
│       ├── LibMain.h           # GigPerformerAPI subclass
│       ├── LibMain.cpp
│       ├── Version.h           # PSL_VERSION_STRING
│       ├── bridge/             # Logger, Dispatcher, GpHost, ExtensionContext, …
│       └── protocol/           # MessageProtocol, PV/PC/PS/MS parsers, DataClient
├── tests/                      # GoogleTest, brought in via FetchContent
│   ├── CMakeLists.txt
│   └── unit/
│       └── test_sanity.cpp     # phase-0 placeholder
├── docs/
│   └── GP_EXTENSION_PLAN.md    # canonical design doc (kept in sync with JS repo)
├── cmake/                      # helper modules (currently empty)
└── .github/workflows/ci.yml    # Windows configure + build + ctest
```

---

## License

MIT. See [`LICENSE`](LICENSE).

The `gp-sdk` source pulled in via `add_subdirectory` retains its own
licence (see the SDK repo's `LICENSE.md`).
