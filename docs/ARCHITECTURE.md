# OpenRide architecture

## Scope (current phase)
Engine foundation + keyboard/mouse "viewer mode". No Cesium, no Bluetooth trainer,
no external map plugins. Test world: one ~400 m stadium-shaped closed circuit.

## Repository layout (target)
```
OpenRide.uproject
Source/
  OpenRide/                 UE5 game module (Build.cs, module class)
    Circuit/                ACircuitActor, road mesh generation
    Cyclist/                ABikeActor, anim instance, appearance data
    NPC/                    ANPCCyclist, ARaceManager
    Player/                 ACyclistCharacter (viewer-mode pawn, input, camera)
    Environment/            AGroundActor (done), grass, buildings, sky/lighting
    UI/                     AOpenRideHUD (done), menus
    Core/                   AOpenRideGameMode (default pawn + HUD)
  OpenRide.Target.cs
  OpenRideEditor.Target.cs
Config/                     DefaultEngine/Game/Input .ini (Enhanced Input enabled)
Content/                    UE assets (binary files tracked via Git LFS, see .gitattributes)
core/                       Engine-independent pure logic + unit tests
  circuit/                  Stadium loop math (testable on ARM Linux, no UE)
  road/                     Road + lane-marking ribbon geometry (no UE)
  anim/                     Cadence / lean / sprint model for the rider animation (no UE)
  bike/                     Wheel spin / steering / lean model for the bike (no UE)
docs/                       ARCHITECTURE, ASSETS, BLOCKERS, CYCLIST_PIPELINE (rider/bike asset spec), ANIMATION_BLUEPRINT_SPEC (ABP_Cyclist)
```

## Project scaffold notes
- Targets UE 5.4 (`EngineAssociation` in `OpenRide.uproject`); change it if you use another 5.x.
- Module `OpenRide` is the primary game module; log category `LogOpenRide` is declared in `OpenRide.h`.
- Not compile-tested: UE cannot build on the ARM dev machine. First PC build may need small fixes.
- Git LFS must be installed (`git lfs install`) before adding uasset/umap/fbx/png/wav files.

## Core module (engine-independent)
- `core/circuit/StadiumCircuit.{h,cpp}`: `FStadiumCircuit` (default 400 m, R = 30 m; straight length derived).
  `SampleAtDistance(m)` returns position, unit tangent and wrapped distance; `GenerateSamples(N)` for spline points.
- Local frame: start at origin heading +X, loop runs counter-clockwise, metres.
- UE reuse: `Source/OpenRide/Circuit/CircuitCoreBridge.cpp` `#include`s the core .cpp (UBT only compiles files inside the module); `ACircuitActor` includes the core header by relative path.
- `core/road/RoadMesh.{h,cpp}`: `BuildLoopRibbon` / `BuildDashedRibbon` return `FRoadMesh` (vertices, UVs, up-facing triangles) in metres.
  `ACircuitActor::RebuildRoadMesh()` copies them into two `UProceduralMeshComponent`s (asphalt, markings: 2 solid edge lines + dashed centre line); `RoadMeshBridge.cpp` compiles the core source into the module.
  Materials are placeholders (engine WorldGridMaterial) until real assets are added; UV.V is in metres.
- Tests: `cd core/road && cmake -S . -B build && cmake --build build && ./build/RoadMeshTests`
- `core/movement/CyclistMovement.h`: header-only `StepMovement` speed model (throttle/brake/coast, configurable max speed); included directly by `ACyclistCharacter`.
- `core/lap/LapTracker.h`: header-only `FLapTracker` (total distance, completed laps, lap/last/best time); fed by `ACyclistCharacter` from the unwrapped movement advance.
- `core/anim/CyclistAnimModel.h`: header-only `StepAnimModel` derives cadence (fixed gearing), lean (banking from speed x yaw rate) and sprint (hysteresis) from motion; used by `UCyclistAnimInstance`.
- `core/bike/BikeModel.h`: header-only `StepBikeModel` (wheel spin = v/r, bicycle-kinematics steering, banking lean); used by `ABikeActor`.
- Tests: `cd core/bike && cmake -S . -B build && cmake --build build && ./build/BikeModelTests`
- Tests: `cd core/anim && cmake -S . -B build && cmake --build build && ./build/CyclistAnimModelTests`
- Tests: `cd core/lap && cmake -S . -B build && cmake --build build && ./build/LapTrackerTests`
- Tests: `cd core/movement && cmake -S . -B build && cmake --build build && ./build/CyclistMovementTests`
- Tests: `cd core/circuit && cmake -S . -B build && cmake --build build && ./build/StadiumCircuitTests`

## Class overview (planned)
| Class | Responsibility |
|-------|----------------|
| `ACircuitActor` (done, uncompiled) | Owns a `USplineComponent` built from circuit math; `GetTransformAtDistance(Meters)`; generates road + lane marking meshes |
| `AGroundActor` (done, uncompiled) | Grass ground plane just under the road, fitted to the circuit bounds + margin; `UBoxComponent` playable bounds, `IsInsideBounds()`; placeholder grid material |
| `ACyclistCharacter` (done, uncompiled) | `APawn` following the circuit by distance; Enhanced Input built in code (no .uasset): W/S/Up/Down move, mouse look, scroll zoom, `C` cycles chase / first-person / free-fly camera (`ECyclistCameraMode`); in free-fly the rider is parked and WASD+E/Q fly the camera along the view direction; speed from `core/movement` |
| `AOpenRideHUD` (done, uncompiled) | `AHUD` canvas overlay (UI/): speed km/h, distance, lap counter, lap/last/best time; reads `ACyclistCharacter` getters |
| `AOpenRideGameMode` (done, uncompiled) | Core/: default pawn = `ACyclistCharacter`, HUD = `AOpenRideHUD`; set as `GlobalDefaultGameMode` in DefaultEngine.ini |
| `ABikeActor` (done, uncompiled) | Cyclist/: measures its own speed + yaw rate, runs `core/bike`; rolls `LeanPivot` (lean), exposes wheel angle / steer for `UBikeAnimInstance` (`ABP_Bike`); `AttachRider()` snaps rider to `Saddle`, `GetSocketWorldLocation()` for Hand*/Foot* IK targets |
| `UCyclistAnimInstance` (done, uncompiled) | Cyclist/: measures owner speed + yaw rate, runs `core/anim`, exposes Speed, Cadence, Lean, bSprinting, bPedaling to `ABP_Cyclist` (spec in docs/ANIMATION_BLUEPRINT_SPEC.md) |
| `UCyclistAppearanceData` | Data asset: jersey, helmet, bike colour |
| `ANPCCyclist` | Same rider+bike classes, follows spline at target speed |
| `ARaceManager` | Spawns and manages NPCs |

## Conventions
- Epic coding standard (UpperCamelCase, `F/U/A/E` prefixes).
- Small single-responsibility classes; comments explain *why*.
- Pure logic lives in `core/` without UE dependencies so it can be unit-tested on the Pi.
- Every third-party asset/library must be permissively licensed and listed in `ASSETS.md`.
- Unreal cannot be built on the Raspberry Pi dev machine: prefer simple, conventional UE5 patterns.

## Roadmap
1. Phase 0: foundation (docs, project scaffold, README)
2. Phase 1: 400 m circuit (math, spline actor, road, ground)
3. Phase 2: player, movement model, cameras, HUD
4. Phase 3: cyclist and bike models, animation
5. Phase 4: NPC cyclists
6. Phase 5: environment (grass, buildings, sky)
7. Phase 6: polish, settings, CI
8. Later: Bluetooth smart trainer, then map/terrain generation
