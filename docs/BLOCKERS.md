# Blockers

Record anything that prevents a task from being completed, then move on to the next task.

Format: `- [date] Task: blocker description (what is needed to unblock)`

- [2026-11-22] Phase B Diagnose tasks 1-5 (Windows build, UBT Log.txt, .rsp files, target settings, blank UE 5.8 project): need a Windows machine with UE 5.8; cannot run on the Raspberry Pi.
- [2026-11-22] Phase B "Rebuild and verify the C1083 error is gone": needs a Windows UE 5.8 build; cannot run on the Raspberry Pi.
- [2026-11-22] Phase B Diagnose/Remaining tasks needing a Windows UE 5.8 build (target-settings comparison, blank-project diff, MSVC flag check, re-run build, Game target): cannot run on the Raspberry Pi.
- [2026-11-22] Phase B "Verify ProceduralMeshComponent plugin" / "Verify EnhancedInput, UMG, Slate" under 5.8: static check OK (.uproject enables EnhancedInput + ProceduralMeshComponent, names match Build.cs) but module renames in 5.8 need a Windows build. Also note OpenRide.uproject has EngineAssociation "5.4" while the task targets 5.8; confirm on Windows.
- [2026-11-22] Phase B "Verify the core headers compile under MSVC": static g++ warning check done and clean; the actual MSVC /W4 / UE flag compile still needs a Windows build.
- [2026-11-22] Phase B "Environment noise" tasks (UbaSessionServer access denied, VS 2022 14.44 / Windows SDK 10.0.22621 vs UE 5.8 requirements, regenerate project files and build .sln): Windows-only; need a Windows machine with UE 5.8 and VS 2022.
- [2026-11-23] Re-runs (1st-737th): every unchecked TASKS.md item still needs Windows + UE 5.8 + VS 2022; nothing could be ticked on the Pi. Unblock via scripts/build-win.ps1 and docs/BUILD_CHECK.md. Update this single line instead of adding a new one per run.
