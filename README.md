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
> The GPScript prefix `psl_` comes from the extension's product XML `name`
> attribute (see `LibMain.cpp::GetProductDescription`). The friendly label
> shown in GP's extensions list is set via the `Description` field; the
> short XML name `psl` exists purely to make GPScript identifiers terse.

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
build/bin/Debug/PreSonusStudioLive.dll
```

To install into Gig Performer's Extensions folder (requires admin or
write access to `C:\Users\All Users\Gig Performer\Extensions\`):

```powershell
cmake --install build --config Debug --component dev
```

---

## Manual smoke test (Phase 0 acceptance)

1. Build Debug as above.
2. Copy `build/bin/Debug/PreSonusStudioLive.dll` into
   `C:\Users\All Users\Gig Performer\Extensions\`.
3. Start Gig Performer.
4. Open **Options → Extensions** — `PreSonus StudioLive` should appear
   in the list with a version of `1.0.0-phase0` and a description. Tick
   the box to enable it. Restart GP if prompted.
5. Create a scratch rackspace, open the GPScript window for the rackspace,
   and add:

    ```gigperformer
    Initialization
        Print(psl_Version())
    End
    ```

6. Save the rackspace. The GP log should print
   `1.0.0-phase0`.

Until step 5 passes, Phase 0 is not actually done — only the local build
is verified.

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
│       └── bridge/             # (Phase 1+) GpHost, Dispatcher, ConfigStore...
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
