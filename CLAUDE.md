# OpenRide: project rules for Claude Code

## Project

OpenRide is an open-source cycling simulation engine/game. Long-term, it connects to a
Bluetooth smart home trainer and renders a world map. RIGHT NOW the only goal is a solid,
working engine foundation with a keyboard/mouse "viewer mode".

## Hard constraints (current phase)

- NO Cesium, NO Cesium Ion, NO external map/terrain plugins.
- NO Bluetooth / home trainer integration yet. Input = keyboard + mouse only.
- Test world = ONE simple closed circuit, ~400 m rounded loop (stadium/oval shape).
- Everything must be open-source friendly: use only assets/libs with permissive licenses,
  and document each one in docs/ASSETS.md.

## Workflow rules

- Work ONLY on the `unreal` branch. Never touch `main`.
- One task from TASKS.md per run: the first unchecked `- [ ]` item.
- Before ticking a task: code compiles (if the toolchain is available), tests/docs updated,
  README/docs reflect the change.
- Commit small and often, with clear messages (`feat:`, `fix:`, `docs:`, `refactor:`).
- When a task is done, change `- [ ]` to `- [x]` and add a one-line note under it.
- If a task is too big, split it into sub-tasks in TASKS.md instead of half-doing it.
- If blocked, write the blocker in docs/BLOCKERS.md and move on to the next task.

## Environment notes

- Development machine is a Raspberry Pi (ARM Linux): Unreal Engine CANNOT build or run here.
  Write code, configs and docs carefully, since you cannot compile-test UE code locally.
  Any change under `Source/` MUST be listed (files + what changed) in the commit message or
  docs/BUILD_CHECK.md for a manual Windows build check before merge.
  Prefer simple, conventional UE5 C++ patterns to minimize errors.
- Keep pure logic (circuit math, physics helpers) in engine-independent files/tests where
  possible, so it can be unit-tested on the Pi.

## Code style

- Unreal C++: follow Epic coding standard (UpperCamelCase types, `F/U/A/E` prefixes).
- Small, single-responsibility classes. Comment the *why*, not the *what*.
- Include convention (Unreal module): use module-root-relative paths, e.g. `#include "Circuit/CircuitActor.h"`
  (resolved via `PublicIncludePaths.Add(ModuleDirectory)` in `OpenRide.Build.cs`). Do not use bare
  same-folder includes for other folders' headers; the `.h` matching a `.cpp` comes first.
- Module/folder layout is defined in docs/ARCHITECTURE.md (keep it updated).

## Definition of done for the whole phase

A playable level where the player (keyboard/mouse) can ride a detailed cyclist on a bike
around a 400 m loop, with NPC cyclists using the same model, and an environment of grass,
road, houses and buildings.

- Markdown must pass markdownlint: put a blank line before and after every heading, list and code fence, and give every code fence a language.
