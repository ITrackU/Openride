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
- [x] Verify the core headers compile under MSVC with UE's flags (check for `/WX`, shadowing, `check`/`min`/`max` macro clashes, RTTI/exceptions assumptions).
  - Static check on the Pi (g++ -Wall -Wextra -Wshadow -Wconversion): fixed an unused lambda and 3 int->float conversions; no UE macro/name clashes, no RTTI use, only `throw` (exceptions enabled). Real MSVC compile still needs Windows.
- [x] Make sure core sources are either header-only or compiled into the module (decide: `core/*.cpp` pulled in via Build.cs vs. a separate static library).
  - Decision: header-only except 2 `.cpp` files, compiled via the bridge `.cpp` includes (no static library); documented in docs/ARCHITECTURE.md. Not compile-tested (needs Windows).

### Remaining build errors

- [ ] Re-run the build after the fixes above; fix each new error one at a time, committing per fix.
- [x] Check `CircuitCoreBridge.cpp` and `RoadMeshBridge.cpp` (very small files) compile and are not empty stubs that break linking.
  - Static check on the Pi: not stubs, each just `#include`s a real core `.cpp` (90/86 lines; `#pragma once` headers, no duplicate definitions). Compile still needs Windows. Risk found: `StadiumCircuit.cpp:20` uses `throw`, but UE disables C++ exceptions by default (C4530); see new task below.
- [x] Resolve `throw std::invalid_argument` in `core/circuit/StadiumCircuit.cpp` under UE (set `bEnableExceptions = true` in `OpenRide.Build.cs`, or replace the throw with a UE-safe error path while keeping the Pi unit tests valid).
  - Set `bEnableExceptions = true` in `OpenRide.Build.cs` (keeps the throw and the Pi tests valid); not compile-tested, needs Windows.
- [ ] Verify `ProceduralMeshComponent` plugin is enabled and its module name matches in `.uproject` and Build.cs under UE 5.8 (module/plugin renames happen between versions).
- [ ] Verify `EnhancedInput`, `UMG`, `Slate`, `SlateCore` dependencies resolve in 5.8.
- [ ] Build the `OpenRide` Game target (not just Editor) for Win64 Development and confirm it succeeds.

### Environment noise (non-blocking, but check)

- [ ] Investigate `UbaSessionServer ... Access is denied` on `C:\ProgramData\Epic\UnrealBuildAccelerator`; try running the build once as Administrator or clearing the `sessions`/`cas` folders; confirm it does not affect the build result.
- [ ] Confirm the VS 2022 toolchain (14.44) and Windows 10.0.22621 SDK match what UE 5.8 officially requires; install the recommended workloads from `.vsconfig` if not.
- [ ] Regenerate project files (`OpenRide.uproject` -> Generate Visual Studio project files) and confirm the `.sln` builds from the IDE too.

### Prevent regressions

- [x] Add a `scripts/build-win.ps1` that runs the exact Build.bat command, tees to `build.log`, and prints only error lines.
  - Added `scripts/build-win.ps1` (`-EngineDir`/`UE_ROOT`, `-Target`); not run, needs Windows. Uses the standard `Build.bat <Target> Win64 Development -Project=...` form.
- [x] Add a CI job (GitHub Actions, Windows runner with UE or a documented manual step) or a documented local check so the Unreal module is compiled before merge.
  - Documented manual Windows check in docs/BUILD_CHECK.md (free runners can't run UE); not run, needs Windows.
- [x] Update `CLAUDE.md`: the Linux automation cannot compile the Unreal module; any change under `Source/` must be listed for a manual Windows build check.
  - Added to CLAUDE.md environment notes; points to docs/BUILD_CHECK.md.
- [x] Add `build.log` to `.gitignore`.
  - Added under a comment; `scripts/build-win.ps1` writes `build.log` at the repo root. No `Source/` change.

## Phase C: Fix markdownlint errors (75 errors, 10 files)

Context: the CI markdown job fails with MD022, MD032, MD031, MD040 and MD025.
Goal: `npx markdownlint-cli2 "**/*.md"` exits with 0 errors.

### Auto-fixable rules

- [ ] Run `npx markdownlint-cli2 --fix "**/*.md"` from the repo root and review the diff.
- [ ] Confirm `CLAUDE.md` has a blank line before and after every heading and list (MD022, MD032).
- [ ] Confirm `CONTRIBUTING.md` and `README.md` have blank lines around headings, lists and code fences (MD022, MD032, MD031).
- [ ] Confirm `TASKS.md` itself passes: blank line around every `###` heading and every list block.
- [ ] Confirm `docs/ARCHITECTURE.md` and `docs/CYCLIST_PIPELINE.md` pass MD022, MD032 and MD031.

### Manual fixes

- [ ] MD040: add a language to the bare code fence at `README.md:40` (use `text` if it is not code).
- [ ] MD040: add a language to the bare code fence at `docs/ARCHITECTURE.md:8` (use `text`).
- [ ] MD040: add a language to the bare code fence at `docs/ANIMATION_BLUEPRINT_SPEC.md:26` (use `text`).
- [ ] MD025: in `docs/ANIMATION_BLUEPRINT_SPEC.md:55`, change the second top-level `#` heading to `##`, or delete it if it duplicates the title.
- [ ] MD031: make sure every fenced code block in `docs/ANIMATION_BLUEPRINT_SPEC.md` has a blank line before and after it.

### Docs accuracy

- [ ] Update `docs/ARCHITECTURE.md:36` from "Targets UE 5.4" to "Targets UE 5.8".
- [ ] Search all `*.md` files for other mentions of UE 5.4 and update them to 5.8.

### Prevent regressions

- [ ] Add this rule to the Workflow rules in `CLAUDE.md`: Markdown must pass markdownlint, with blank lines around headings, lists and code fences, and a language on every code fence.
- [ ] Add a `.markdownlint-cli2.jsonc` if missing, so local and CI lint use the same config.
- [ ] Bump the actions in `.github/workflows/*.yml` (`actions/checkout`, `DavidAnson/markdownlint-cli2-action`) to versions that run on Node 24, to remove the Node 20 deprecation warning.
- [ ] Run `npx markdownlint-cli2 "**/*.md"` one last time and confirm 0 errors, then commit on the `unreal` branch with the message "Fix markdownlint errors"
.
