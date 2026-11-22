# OpenRide: engine tasks (unreal branch)

## Phase 0: Foundation
- [x] Create docs/ARCHITECTURE.md: module layout, class overview, coding conventions,
      and a short roadmap. Create docs/ASSETS.md (license tracking) and docs/BLOCKERS.md.
      Note: created the three docs in docs/; layout includes a planned engine-independent core/ folder.
- [x] Scaffold a UE5 C++ project: OpenRide.uproject, Source/OpenRide module
      (Build.cs, Target.cs files, module class), .gitignore for Unreal, .gitattributes
      for Git LFS on binary assets (uasset, umap, fbx, png, wav).
      Note: scaffold added (uproject for UE 5.4, module, targets, Config, .gitignore, LFS attributes); not compile-tested on the Pi.
- [x] Add README.md (vision, current phase, build instructions for a PC, license choice
      suggestion with rationale) and CONTRIBUTING.md.
      Note: README.md (MIT suggested, no LICENSE file yet) and CONTRIBUTING.md added.

## Phase 1: Circuit (400 m loop)
- [x] Write an engine-independent circuit math module (C++ or Python prototype with tests):
      generate a closed stadium-shaped loop of total length 400 m (two straights + two
      semicircles), sampling points with position, tangent, and distance along track.
      Include unit tests verifying total length = 400 m +/- 0.1 m.
      Note: C++17 module in core/circuit (R=30 m, straights ~105.75 m); tests pass locally via CMake.
- [x] Create ACircuitActor in UE using a USplineComponent built from that math, with
      configurable width, closed loop, and a function GetTransformAtDistance(float Meters).
      Note: Source/OpenRide/Circuit/CircuitActor.{h,cpp} added (wraps core math via CircuitCoreBridge.cpp); not compile-tested on the Pi.
- [x] Generate road mesh along the spline (procedural mesh or spline mesh components)
      with a basic road material placeholder and lane markings.
      Note: core/road geometry (tested via CMake) feeds two ProceduralMesh components in ACircuitActor; placeholder grid material; not compile-tested on the Pi.
- [x] Add ground plane/landscape placeholder with a grass material and bounds around the loop.
      Note: AGroundActor (Environment/) fits a plane + playable-bounds box to the circuit; placeholder grid material for grass; not compile-tested on the Pi.

## Phase 2: Player and camera (viewer mode)
- [x] Create ACyclistCharacter (or Pawn) with Enhanced Input: WASD/arrows to move along
      the circuit, mouse look, scroll to zoom camera, key to toggle camera modes.
      Note: Source/OpenRide/Player/CyclistCharacter.{h,cpp} (APawn; input actions/mapping created in code; placeholder constant speed); not compile-tested on the Pi.
- [x] Implement simple cycling movement model: speed accelerate/decelerate/brake with
      configurable max speed, constrained to the circuit spline (follow by distance).
      Note: core/movement (header-only, tested via CMake) drives ACyclistCharacter; W accelerates, S brakes, release coasts; not compile-tested on the Pi.
- [x] Camera system: third-person chase cam, first-person cam, free-fly debug cam,
      switchable at runtime.
      Note: ECyclistCameraMode in ACyclistCharacter; `C` cycles Chase/FirstPerson/FreeFly (WASD+E/Q fly, rider parked); not compile-tested on the Pi.
- [x] Create a minimal HUD: speed (km/h), distance, lap counter, lap time.
      Note: AOpenRideHUD (canvas) + AOpenRideGameMode + core/lap FLapTracker (tested via CMake); not compile-tested on the Pi.

## Phase 3: Cyclist and bike models
- [x] Define the cyclist asset pipeline in docs/: skeletal mesh requirements, skeleton
      naming, animation list (idle, pedaling cadence-driven, coasting, standing sprint),
      and how rider + bike attach via sockets.
      Note: docs/CYCLIST_PIPELINE.md added (meshes, skeletons, sockets, animations, anim input contract); docs only.
- [x] Implement a cadence-driven pedaling animation system in C++ (AnimInstance exposing
      Cadence, Speed, Lean) with an Animation Blueprint spec documented for the PC side.
      Note: UCyclistAnimInstance (Cyclist/) + core/anim model (tested via CMake) + docs/ANIMATION_BLUEPRINT_SPEC.md; not compile-tested on the Pi.
- [x] Create ABikeActor with wheel rotation tied to speed, steering/lean visual, and
      sockets for rider hands/feet/saddle.
      Note: ABikeActor + UBikeAnimInstance (Cyclist/) + core/bike model (tested via CMake) + ABP_Bike spec; not compile-tested on the Pi.
- [x] Add a customization data asset (UCyclistAppearanceData): jersey color, helmet, bike
      color, so every cyclist is the same base model with different appearance.
      Note: UCyclistAppearanceData (Cyclist/) applies colours via MIDs + core/appearance seeded randomiser (tested via CMake); not compile-tested on the Pi.

## Phase 4: NPC cyclists
- [x] Create ANPCCyclist reusing the same rider+bike classes, following the circuit spline
      at a configurable target speed with slight lane offset.
      Note: ANPCCyclist (NPC/) + core/npc follower model (tested via CMake); rider mesh/ABP and bike class assigned in a Blueprint subclass; not compile-tested on the Pi.
- [x] Spawner/manager (ARaceManager): spawn N NPCs with randomized appearance, speeds,
      and start offsets. Basic overtaking/avoidance via lane offset.
      Note: ARaceManager (NPC/) + core/race plan/avoidance model (tested via CMake); player treated as lane-0 rider; not compile-tested on the Pi.

## Phase 5: Environment
- [x] Procedural/instanced grass and foliage around the loop (HISM) with density settings.
      Note: AFoliageActor (Environment/) + core/foliage jittered-grid scatter (tested via CMake); engine cone/grid placeholders; not compile-tested on the Pi.
- [ ] Modular house/building generator or placeholder meshes placed around the circuit
      (several variants), with a data-driven placement actor.
- [ ] Add sky/lighting setup (directional light, sky atmosphere, post-process) and a
      day-time parameter.

## Phase 6: Polish and tooling
- [ ] Settings menu (resolution, graphics quality, key bindings) and pause menu.
- [ ] Automated checks: a GitHub Actions workflow running the pure-logic unit tests and
      markdown/link lint (no UE build in CI).
- [ ] Update README with screenshots section, roadmap, and next-phase plan
      (Bluetooth trainer, then map generation).
