# Animation Blueprint spec: `ABP_Cyclist`

To be built on the PC (UE Editor); the C++ side is `UCyclistAnimInstance`
(`Source/OpenRide/Cyclist/`), maths in `core/anim/CyclistAnimModel.h`.
Assets and animation list: see [CYCLIST_PIPELINE.md](CYCLIST_PIPELINE.md).

## Setup
1. Create `ABP_Cyclist` with parent class `CyclistAnimInstance`, target skeleton `SKEL_Cyclist`.
2. Assign it as the Anim Class of the rider skeletal mesh component (in `ABikeActor`).
3. The instance reads motion from its owning actor (position change + yaw), so no event-graph
   logic is needed: do NOT compute speed/cadence/lean in the Blueprint.

## Variables (read-only, set by C++ every frame)
| Variable | Type | Meaning |
|----------|------|---------|
| `Speed` | float | Forward speed, m/s |
| `Cadence` | float | Crank rpm; 0 when coasting/stopped |
| `Lean` | float | -1 (left) .. +1 (right) |
| `bPedaling` | bool | Legs driving cranks |
| `bSprinting` | bool | Out of the saddle (cadence >= 100 rpm, off below 90) |

Tuning (instance defaults, editable in the ABP class defaults): `GearDevelopmentMeters` (6.5),
`MaxCadenceRpm` (120), `SprintCadenceRpm` (100), `MaxLeanDegrees` (35).

## Anim graph
```
[Locomotion state machine] -> [Apply Additive: Lean] -> [Two-Bone IK hands/feet] -> Output Pose
```

### Locomotion state machine (blend time 0.2 s)
| State | Asset | Enter condition |
|-------|-------|-----------------|
| `Idle` | `A_Cyc_Idle` | `Speed < 0.5` |
| `Pedal` | `A_Cyc_Pedal` | `bPedaling && !bSprinting` |
| `Coast` | `A_Cyc_Coast` | `Speed >= 0.5 && !bPedaling` |
| `StandSprint` | `A_Cyc_StandSprint` | `bPedaling && bSprinting` |

Pedal and StandSprint: use a Sequence Player with the play rate driven by
`GetPedalPlayRate(<clip length in seconds>)`. The clips are 1 crank turn long, so legs stay in sync
with the cranks (e.g. 90 rpm on a 1 s clip -> rate 1.5).

### Lean layer
Blend Poses by `Lean`: `Lean < 0` -> additive `A_Cyc_Lean_L` with alpha `-Lean`;
`Lean > 0` -> additive `A_Cyc_Lean_R` with alpha `Lean`. Apply on top of every state.

### IK
Two-Bone IK (or Control Rig) for hands to `HandLeft/Right` and feet to `FootLeft/Right` sockets
on `SK_Bike`, so pedals and handlebars drive the limbs.

## Notes
- Gearing is a single fixed ratio for now (cadence is proportional to speed). A future
  Bluetooth trainer will supply the real cadence; `UCyclistAnimInstance` is where it will plug in.
- The anim logic has unit tests: `cd core/anim && cmake -S . -B build && cmake --build build && ./build/CyclistAnimModelTests`.
