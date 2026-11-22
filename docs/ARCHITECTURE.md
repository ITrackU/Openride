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
    Environment/            AGroundActor (done), AFoliageActor (done), ABuildingPlacerActor (done), ASkyLightingActor (done)
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
  appearance/               Seeded random jersey/shorts/helmet/frame colours (no UE)
  npc/                      NPC speed + lane-offset follower model (no UE)
  menu/                     Pause-menu selection + hit-test model (no UE)
  race/                     spawn planning + lane avoidance for the NPC field (no UE)
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
- `core/settings/SettingsModel.h`: header-only settings model (resolution list + nearest match, quality levels 0..3 matching `sg.*`, `FKeyBindings` with swap-on-conflict rebinding); to be used by the settings menu (UI/).
- `core/menu/MenuModel.h`: header-only vertical menu model (wrap-around selection skipping disabled items, centred button layout, pixel hit-test); used by the pause menu in `AOpenRideHUD`.
- Tests: `cd core/menu && cmake -S . -B build && cmake --build build && ./build/MenuModelTests`
- `core/anim/CyclistAnimModel.h`: header-only `StepAnimModel` derives cadence (fixed gearing), lean (banking from speed x yaw rate) and sprint (hysteresis) from motion; used by `UCyclistAnimInstance`.
- `core/bike/BikeModel.h`: header-only `StepBikeModel` (wheel spin = v/r, bicycle-kinematics steering, banking lean); used by `ABikeActor`.
- `core/appearance/AppearanceModel.h`: header-only `RandomAppearance(seed)` (xorshift PRNG + HSV palette rules); used by `UCyclistAppearanceData::MakeRandom`.
- `core/npc/NpcFollower.h`: header-only `StepNpcFollower` (bounded acceleration towards a target speed, bounded lateral drift towards a target lane); used by `ANPCCyclist`.
- `core/race/RaceModel.h`: header-only `PlanSpawns` (seeded start offsets, speeds, preferred lanes, appearance seeds) and `DecideAvoidance` (free-lane overtaking, follow the blocker if boxed in); used by `ARaceManager`.
- `core/foliage/FoliageScatter.h`: header-only `ScatterFoliage` (seeded jittered grid, density per m², road-clearance cull via `DistanceToLoop`, instance cap); used by `AFoliageActor`.
- `core/buildings/BuildingPlacer.h`: header-only `PlaceBuildings` (walks the loop, outside-only, setback from the road, weighted variants, no overlaps; seeded); used by `ABuildingPlacerActor`.
- `core/sky/SkyModel.h`: header-only `ComputeSky(hours)` (sinusoidal sun elevation/azimuth, sun intensity + colour temperature, ambient level, exposure bias); used by `ASkyLightingActor`.
- Tests: `cd core/sky && cmake -S . -B build && cmake --build build && ./build/SkyModelTests`
- Tests: `cd core/buildings && cmake -S . -B build && cmake --build build && ./build/BuildingPlacerTests`
- Tests: `cd core/foliage && cmake -S . -B build && cmake --build build && ./build/FoliageScatterTests`
- Tests: `cd core/race && cmake -S . -B build && cmake --build build && ./build/RaceModelTests`
- Tests: `cd core/npc && cmake -S . -B build && cmake --build build && ./build/NpcFollowerTests`
- Tests: `cd core/appearance && cmake -S . -B build && cmake --build build && ./build/AppearanceModelTests`
- Tests: `cd core/bike && cmake -S . -B build && cmake --build build && ./build/BikeModelTests`
- Tests: `cd core/anim && cmake -S . -B build && cmake --build build && ./build/CyclistAnimModelTests`
- Tests: `cd core/lap && cmake -S . -B build && cmake --build build && ./build/LapTrackerTests`
- Tests: `cd core/settings && cmake -S . -B build && cmake --build build && ./build/SettingsModelTests`
- Tests: `cd core/movement && cmake -S . -B build && cmake --build build && ./build/CyclistMovementTests`
- Tests: `cd core/circuit && cmake -S . -B build && cmake --build build && ./build/StadiumCircuitTests`

## Class overview (planned)
| Class | Responsibility |
|-------|----------------|
| `ACircuitActor` (done, uncompiled) | Owns a `USplineComponent` built from circuit math; `GetTransformAtDistance(Meters)`; generates road + lane marking meshes |
| `AGroundActor` (done, uncompiled) | Grass ground plane just under the road, fitted to the circuit bounds + margin; `UBoxComponent` playable bounds, `IsInsideBounds()`; placeholder grid material |
| `ACyclistCharacter` (done, uncompiled) | `APawn` following the circuit by distance; Enhanced Input built in code (no .uasset): W/S/Up/Down move, mouse look, scroll zoom, `C` cycles chase / first-person / free-fly camera (`ECyclistCameraMode`); in free-fly the rider is parked and WASD+E/Q fly the camera along the view direction; speed from `core/movement` |
| `AOpenRideHUD` (done, uncompiled) | `AHUD` canvas overlay (UI/): speed km/h, distance, lap counter, lap/last/best time; reads `ACyclistCharacter` getters. Escape toggles a pause menu (Resume / Settings (disabled) / Quit) driven by mouse click or Up/Down/Enter; pauses the game via `SetGamePaused` |
| `AOpenRideGameMode` (done, uncompiled) | Core/: default pawn = `ACyclistCharacter`, HUD = `AOpenRideHUD`; set as `GlobalDefaultGameMode` in DefaultEngine.ini |
| `ABikeActor` (done, uncompiled) | Cyclist/: measures its own speed + yaw rate, runs `core/bike`; rolls `LeanPivot` (lean), exposes wheel angle / steer for `UBikeAnimInstance` (`ABP_Bike`); `AttachRider()` snaps rider to `Saddle`, `GetSocketWorldLocation()` for Hand*/Foot* IK targets |
| `UCyclistAnimInstance` (done, uncompiled) | Cyclist/: measures owner speed + yaw rate, runs `core/anim`, exposes Speed, Cadence, Lean, bSprinting, bPedaling to `ABP_Cyclist` (spec in docs/ANIMATION_BLUEPRINT_SPEC.md) |
| `UCyclistAppearanceData` (done, uncompiled) | Cyclist/: `UDataAsset` with jersey/shorts/helmet/frame colours; `ApplyToRider()` / `ApplyToBike()` set the master-material vector params (`JerseyColor`, `ShortsColor`, `HelmetColor`, `FrameColor`) via dynamic material instances; `MakeRandom(Outer, Seed)` for NPCs |
| `ANPCCyclist` (done, uncompiled) | NPC/: `AActor` with a rider `USkeletalMeshComponent`; spawns `BikeClass` (default `ABikeActor`) at BeginPlay, attaches the rider to the Saddle socket, optionally applies `UCyclistAppearanceData`; follows the circuit by distance at `TargetSpeedMps` with a smoothed `LaneOffsetMeters` (right of travel); `core/npc` model |
| `ARaceManager` (done, uncompiled) | NPC/: spawns `NumNPCs` `NPCClass` actors (deferred spawn) with seeded speed/lane/start offset and `UCyclistAppearanceData::MakeRandom`; each tick (post-update) feeds every NPC's `TargetSpeedMps`/`LaneOffsetMeters` from `core/race` avoidance, treating the player as a lane-0 rider |
| `AFoliageActor` (done, uncompiled) | Environment/: one `UHierarchicalInstancedStaticMeshComponent` of grass tufts scattered by `core/foliage` around the `Circuit` (road clearance, margin, seed); settings `DensityPerSqm`, `MaxInstances`, `CullDistanceMeters`; engine cone + grid material as placeholders; `RebuildFoliage()` |
| `ABuildingPlacerActor` (done, uncompiled) | Environment/: places buildings outside the `Circuit` via `core/buildings`; variants (mesh, material, width/depth/height ranges, weight) come from a `UBuildingSetData` data asset (3 placeholder variants if none assigned); one `UInstancedStaticMeshComponent` per variant, engine cube + grid material as placeholders; `RebuildBuildings()` |
| `ASkyLightingActor` (done, uncompiled) | Environment/: directional sun + `USkyAtmosphereComponent` + real-time-capture sky light + height fog + unbound post-process (auto-exposure); `TimeOfDayHours` (0..24) drives sun rotation/colour/ambient/exposure via `core/sky`; `SetTimeOfDay()`, optional `TimeSpeedHoursPerSecond` to run the clock |
| `UOpenRideSettingsLibrary` (done, uncompiled) | UI/: `UBlueprintFunctionLibrary` over `UGameUserSettings`: offered resolutions (from `core/settings`), `ApplyResolution`, `ApplyQualityLevel` (0..3 -> all `sg.*` groups), `GetQualityLevel`, `SaveSettings`, `LoadAndApplySavedSettings`; used by the upcoming settings screen |

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
