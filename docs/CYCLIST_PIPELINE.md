# Cyclist asset pipeline

Defines what art assets the cyclist and bike must satisfy so that the player and every NPC
share ONE base rider + ONE base bike (only appearance differs, see `UCyclistAppearanceData`).
All source assets must be permissively licensed and listed in `ASSETS.md`.

## Units and orientation
- Unreal units: 1 uu = 1 cm. Export at scale 1.0 (FBX unit = cm).
- Forward = +X, right = +Y, up = +Z. Rider and bike origin at ground contact, between the wheels
  (bike) / at the feet-on-ground point of the neutral pose (rider).
- Rider height ~175 cm; road bike wheelbase ~100 cm, 700c wheel diameter ~68 cm.

## Skeletal meshes
Two separate skeletal meshes, attached at runtime via sockets (not merged), so the same bike
can be reused with any rider and each can be recoloured independently.

| Asset | Name | Notes |
|-------|------|-------|
| Rider mesh | `SK_Cyclist` | Single mesh, <= 30k tris (LOD0), LODs 1-3 at 50/25/10 %. Separate material slots: `Jersey`, `Shorts`, `Skin`, `Helmet`, `Shoes`. |
| Rider skeleton | `SKEL_Cyclist` | UE Mannequin-compatible humanoid hierarchy (pelvis, spine_01-03, neck, head, clavicle/upperarm/lowerarm/hand, thigh/calf/foot/ball, `_l`/`_r` suffix) so UE retargeting works. |
| Bike mesh | `SK_Bike` | <= 20k tris LOD0. Material slots: `Frame`, `Components`, `Tyres`. |
| Bike skeleton | `SKEL_Bike` | Bones: `root`, `frame`, `fork` (steering), `wheel_front`, `wheel_rear`, `crank`, `pedal_l`, `pedal_r`. |

Rules: all bones skinned with max 4 influences; no scaled bones; T-pose or A-pose reference;
rider rest pose = seated on the saddle, hands on hoods, so it fits the bike with no offset.

## Sockets
Bike skeleton (`SK_Bike`):
| Socket | Parent bone | Used by |
|--------|-------------|---------|
| `Saddle` | `frame` | Rider pelvis attach point |
| `HandLeft`, `HandRight` | `fork` | Hand IK targets (follow steering) |
| `FootLeft`, `FootRight` | `pedal_l` / `pedal_r` | Foot IK targets (follow pedals) |
| `CameraFirstPerson` | `frame` | Optional, first-person eye position (otherwise taken from rider head) |

Rider skeleton (`SK_Cyclist`):
| Socket | Parent bone | Used by |
|--------|-------------|---------|
| `Helmet` | `head` | Optional separate helmet mesh |
| `CameraHead` | `head` | First-person camera |

Attachment: `ABikeActor` owns the bike mesh; the rider mesh is attached to the bike's `Saddle`
socket (`SnapToTarget`, rider pelvis aligned to socket). Hands/feet are placed with Two-Bone IK
(or Control Rig) to the `Hand*`/`Foot*` sockets, so pedals and handlebars drive the limbs
rather than baking positions into animations.

## Animation list
All animations authored at 30 fps, on `SKEL_Cyclist`, rider in bike pose.

| Animation | Type | Driven by | Description |
|-----------|------|-----------|-------------|
| `A_Cyc_Idle` | Loop | - | Stopped, one foot down or balanced on the bike, subtle breathing. |
| `A_Cyc_Pedal` | Loop, 1 cycle = 1 crank turn | `Cadence` (rpm) | Seated pedalling. Play rate = Cadence / 60 cycles per second, so legs stay in sync with the crank. |
| `A_Cyc_Coast` | Loop | - | Seated, cranks level (3 and 9 o'clock), legs still. |
| `A_Cyc_StandSprint` | Loop, 1 cycle = 1 crank turn | `Cadence` | Out-of-saddle pedalling, bike rocking side to side. |
| `A_Cyc_Lean_L` / `A_Cyc_Lean_R` | Additive pose | `Lean` (-1..1) | Body lean into corners, blended additively. |

Locomotion state machine: `Idle` -> `Pedal` (speed > 0.5 m/s and cadence > 0) <-> `Coast`
(cadence == 0); `Pedal` -> `StandSprint` when `bSprinting` (cadence > ~95 rpm or input).
Lean is an additive layer on top of all states. Blend times ~0.2 s.

## Animation inputs (contract with C++)
`UCyclistAnimInstance` (Phase 3 task 2) exposes: `Speed` (m/s), `Cadence` (rpm),
`Lean` (-1..1, from lateral acceleration / curvature), `bSprinting`. Anything the anim
graph needs beyond these must be added to that class, not computed in the Blueprint.

## Material and customization
- Rider and bike materials use a master material with vector parameters `JerseyColor`,
  `ShortsColor`, `FrameColor`, `HelmetColor`, applied via dynamic material instances
  by `UCyclistAppearanceData`.
- Masks for coloured regions are baked in texture channels (e.g. vertex colour R = jersey),
  not separate meshes.

## Import checklist
1. Export FBX (cm, +X forward, smoothing groups, no cameras/lights).
2. Import skeletal mesh, assign `SKEL_*` skeleton, check sockets exist with names above.
3. Verify the physics asset (simple capsules; not used for gameplay).
4. Add source/author/licence to `docs/ASSETS.md`.
5. Store `.uasset`/`.fbx` via Git LFS.
