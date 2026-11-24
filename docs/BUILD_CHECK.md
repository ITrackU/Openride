# Unreal module build check (before merge)

The Unreal module (`Source/OpenRide`) cannot be compiled on free CI runners (no UE install)
or on the Raspberry Pi. CI (`.github/workflows/ci.yml`) therefore only runs the
engine-independent `core/` tests and the docs checks. Compiling the module is a
documented manual step on a Windows machine with UE 5.8 + VS 2022.

## Procedure

1. Check out the branch to merge.
2. Run the Editor target, the one that failed in Phase B:

   ```powershell
   .\scripts\build-win.ps1 -EngineDir "C:\Program Files\Epic Games\UE_5.8"
   ```

3. Run the Game target too:

   ```powershell
   .\scripts\build-win.ps1 -EngineDir "C:\Program Files\Epic Games\UE_5.8" -Target OpenRide
   ```

4. Both must finish with exit code 0 and "No error lines found." The script writes the
   full output to `build.log`; when something fails, fix the **first** error only (later
   ones may be cascades).
5. In the PR description, note which commit was built and the result.

## When it is required

Any change under `Source/`, `*.Build.cs`, `*.Target.cs`, `OpenRide.uproject` or `core/`
headers/sources included by the module.

## Upgrading to automated CI later

A self-hosted Windows runner with UE installed could run the same script as a job. Not
set up yet: it needs a licensed UE install on the runner.
