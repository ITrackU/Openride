## Phase B: Unreal Engine 5.8 build investigation (Windows)

Context: `OpenRideEditor Win64 Development` fails with
`CircuitActor.cpp(1,1): fatal error C1083: Cannot open include file: 'Circuit/CircuitActor.h'`.
Files exist, encoding is plain ASCII, so the module include path is the suspect
(`BuildSettingsVersion.V7` + `EngineIncludeOrderVersion.Latest` no longer add the module root).

### Diagnose
- [ ] Reproduce on Windows: run Build.bat for OpenRideEditor and save the full output to `build.log`; record the first error only (later errors may be cascades).
- [ ] Read `%LOCALAPPDATA%\UnrealBuildTool\Log.txt` and extract the actual `/I` include paths passed to cl.exe for `Module.OpenRide.cpp`; confirm whether `Source/OpenRide` is present.
- [ ] Check `Intermediate/Build/Win64/.../OpenRide/*.rsp` response files to confirm the same include list.
- [ ] Confirm `OpenRideTarget` / `OpenRideEditorTarget` settings (`DefaultBuildSettings`, `IncludeOrderVersion`) against the UE 5.8 defaults for a freshly generated C++ project; note any differences.
- [ ] Generate a throwaway blank UE 5.8 C++ project and compare its `.Build.cs` and Target.cs files with ours.

### Fix include paths
- [x] Add `PublicIncludePaths.Add(ModuleDirectory);` to `OpenRide.Build.cs`.
  - Done on the Pi (not compile-tested); Diagnose tasks 1-5 are blocked on Windows, see docs/BLOCKERS.md.
- [ ] Rebuild and verify the C1083 error for `Circuit/CircuitActor.h` is gone.
- [x] Grep all `Source/OpenRide/**` for `#include "<Folder>/..."` and confirm each resolves from the module root.
  - Static check on the Pi: every non-engine include resolves from the module root, the same folder, or via `../../../core/...` (not compiled; Windows build still needed).
- [x] Decide on a convention (module-root-relative includes vs. same-folder includes) and document it in `CLAUDE.md`.
  - Module-root-relative for cross-folder includes, bare for same-folder siblings; documented in CLAUDE.md and docs/ARCHITECTURE.md.

### Engine-independent core (repo-root `core/`)
- [x] Find every include in `Source/OpenRide` that reaches into `core/` (e.g. `FStadiumCircuit`) and list the paths used.
  - 17 includes, all `../../../core/<dir>/<File>`; list in docs/ARCHITECTURE.md. Note: `CircuitCoreBridge.cpp` and `RoadMeshBridge.cpp` `#include` core `.cpp` files directly.
- [x] Add `core/` to the module include paths in `OpenRide.Build.cs` using `System.IO.Path.GetFullPath(Path.Combine(ModuleDirectory, "../../core"))`.
  - Added via `PublicIncludePaths` in `OpenRide.Build.cs` (not compile-tested on the Pi; needs Windows build check); existing relative includes left unchanged.
- [ ] Verify the core headers compile under MSVC with UE's flags (check for `/WX`, shadowing, `check`/`min`/`max` macro clashes, RTTI/exceptions assumptions).
- [x] Make sure core sources are either header-only or compiled into the module (decide: `core/*.cpp` pulled in via Build.cs vs. a separate static library).
  - Decision: header-only except 2 `.cpp` files, compiled via the bridge `.cpp` includes (no static library); documented in docs/ARCHITECTURE.md. Not compile-tested (needs Windows).

### Remaining build errors
- [ ] Re-run the build after the fixes above; fix each new error one at a time, committing per fix.
- [ ] Check `CircuitCoreBridge.cpp` and `RoadMeshBridge.cpp` (very small files) compile and are not empty stubs that break linking.
- [ ] Verify `ProceduralMeshComponent` plugin is enabled and its module name matches in `.uproject` and Build.cs under UE 5.8 (module/plugin renames happen between versions).
- [ ] Verify `EnhancedInput`, `UMG`, `Slate`, `SlateCore` dependencies resolve in 5.8.
- [ ] Build the `OpenRide` Game target (not just Editor) for Win64 Development and confirm it succeeds.

### Environment noise (non-blocking, but check)
- [ ] Investigate `UbaSessionServer ... Access is denied` on `C:\ProgramData\Epic\UnrealBuildAccelerator`; try running the build once as Administrator or clearing the `sessions`/`cas` folders; confirm it does not affect the build result.
- [ ] Confirm the VS 2022 toolchain (14.44) and Windows 10.0.22621 SDK match what UE 5.8 officially requires; install the recommended workloads from `.vsconfig` if not.
- [ ] Regenerate project files (`OpenRide.uproject` -> Generate Visual Studio project files) and confirm the `.sln` builds from the IDE too.

### Prevent regressions
- [ ] Add a `scripts/build-win.ps1` that runs the exact Build.bat command, tees to `build.log`, and prints only error lines.
- [ ] Add a CI job (GitHub Actions, Windows runner with UE or a documented manual step) or a documented local check so the Unreal module is compiled before merge.
- [ ] Update `CLAUDE.md`: the Linux automation cannot compile the Unreal module; any change under `Source/` must be listed for a manual Windows build check.
- [ ] Add `build.log` to `.gitignore`.
