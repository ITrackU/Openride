# Contributing to OpenRide

Thanks for your interest! Please read [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) first.

## Workflow

- Development happens on the `unreal` branch; do not target `main`.
- Pick the first unchecked task in [TASKS.md](TASKS.md). If it is too big, split it into sub-tasks.
- Commit small and often with prefixes: `feat:`, `fix:`, `docs:`, `refactor:`.
- If you are blocked, record it in [docs/BLOCKERS.md](docs/BLOCKERS.md).

## Code style

- Unreal C++ follows the Epic coding standard (UpperCamelCase, `F/U/A/E` prefixes).
- Small, single-responsibility classes. Comments explain *why*, not *what*.
- Keep pure logic (circuit math, physics helpers) in `core/` without UE dependencies, with unit tests.

## Assets and licenses

- Only permissively licensed assets/libraries (CC0, CC-BY, MIT, Apache-2.0, ...).
- Document each one in [docs/ASSETS.md](docs/ASSETS.md) with source, author and license.
- Binary assets go through Git LFS (see `.gitattributes`).
- No Cesium or external map plugins, and no Bluetooth trainer code in the current phase.

## Scope

Current phase: keyboard/mouse viewer mode on a single ~400 m loop. See the README.
