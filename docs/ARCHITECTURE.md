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
    Player/                 ACyclistCharacter, input, camera modes
    Cyclist/                ABikeActor, anim instance, appearance data
    NPC/                    ANPCCyclist, ARaceManager
    Environment/            grass, buildings, sky/lighting
    UI/                     HUD, menus
  OpenRide.Target.cs
  OpenRideEditor.Target.cs
Config/                     DefaultEngine/Game/Input .ini (Enhanced Input enabled)
Content/                    UE assets (binary files tracked via Git LFS, see .gitattributes)
core/                       Engine-independent pure logic + unit tests
  circuit/                  Stadium loop math (testable on ARM Linux, no UE)
docs/                       ARCHITECTURE, ASSETS, BLOCKERS
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
- Tests: `cd core/circuit && cmake -S . -B build && cmake --build build && ./build/StadiumCircuitTests`

## Class overview (planned)
| Class | Responsibility |
|-------|----------------|
| `ACircuitActor` (done, uncompiled) | Owns a `USplineComponent` built from circuit math; `GetTransformAtDistance(Meters)` |
| `ACyclistCharacter` | Player pawn; follows circuit by distance; cameras and input |
| `ABikeActor` | Wheels, steering/lean visuals, sockets for hands/feet/saddle |
| `UCyclistAnimInstance` | Exposes Cadence, Speed, Lean to the Animation Blueprint |
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
