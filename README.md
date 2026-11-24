# OpenRide

OpenRide is an open-source cycling simulation engine and game built on Unreal Engine 5.
Long-term goal: connect a Bluetooth smart home trainer and ride through a rendered world map.

## Current phase: engine foundation ("viewer mode")

- Input is keyboard + mouse only. No Bluetooth trainer integration yet.
- Press `Esc` to open the pause menu (Resume / Settings / Quit; Settings cycles resolution and quality and rebinds Accelerate/Brake/CycleCamera). Navigate with the mouse or Up/Down + Enter.
- No Cesium, Cesium Ion or external map/terrain plugins.
- Test world: one closed ~400 m stadium-shaped circuit.
- Goal of the phase: ride a detailed cyclist around the loop, with NPC cyclists using the
  same model, in an environment of grass, road, houses and buildings.

Progress is tracked in [TASKS.md](TASKS.md). Design is described in
[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md). Third-party assets and their licenses are listed in
[docs/ASSETS.md](docs/ASSETS.md); Windows build check in [docs/BUILD_CHECK.md](docs/BUILD_CHECK.md); known blockers in [docs/BLOCKERS.md](docs/BLOCKERS.md).

## Screenshots

No screenshots yet: the code is written on a Raspberry Pi where Unreal Engine cannot run.
Once the project has been built on a PC, captures of the circuit, cyclist and pause menu
will be added under `docs/screenshots/` and linked here.

## Roadmap

1. **Engine foundation (current phase)**: closed ~400 m circuit, keyboard/mouse viewer mode,
   detailed cyclist and bike, NPC cyclists, grass/road/houses/buildings environment,
   pause and settings menus, CI for the pure-logic tests. All features are written but
   still need a first compile and playtest on a PC.
2. **Next: Bluetooth smart trainer**: connect a trainer (FTMS over BLE), read power and
   cadence, and map them to the cyclist speed through the engine-independent physics
   helpers; keyboard stays as a fallback input.
3. **Then: map generation**: build rideable roads and terrain from open map data
   (for example OpenStreetMap) without Cesium or other proprietary services, keeping
   every data source and license listed in `docs/ASSETS.md`.

## Building (on a PC)

Requirements: Unreal Engine 5.8 (or another 5.x, see `EngineAssociation` in `OpenRide.uproject`),
a C++ toolchain (Visual Studio 2022 on Windows; clang via the UE toolchain on Linux; Xcode on macOS),
and [Git LFS](https://git-lfs.com/).

```text
git lfs install
git clone <repo-url> && cd Openride
git checkout unreal
```

1. Right-click `OpenRide.uproject` -> "Generate Project Files" (or run
   `GenerateProjectFiles` from your engine install).
2. Open the generated solution and build the `OpenRideEditor` target (Development Editor).
3. Launch the editor by opening `OpenRide.uproject`.

Note: the code is developed on a Raspberry Pi where UE cannot run, so the first PC build
may need small fixes. Please report them as issues.

## License suggestion

We suggest the **MIT License** for code: it is short, permissive, and compatible with
game and commercial use, which lowers the barrier for contributors. Content (art, audio)
should use CC0 or CC-BY, and every asset must be listed in `docs/ASSETS.md`.
Apache-2.0 is a good alternative if an explicit patent grant matters to you.
No `LICENSE` file has been added yet; the project owner should make the final choice.
Note that Unreal Engine itself is under Epic's EULA, not an open-source license.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md).
