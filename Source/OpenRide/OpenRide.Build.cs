using UnrealBuildTool;

public class OpenRide : ModuleRules
{
        public OpenRide(ReadOnlyTargetRules Target) : base(Target)
        {
                PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

                // BuildSettingsVersion.V7 + EngineIncludeOrderVersion.Latest no longer add the
                // module root, so includes like "Circuit/CircuitActor.h" need it explicitly.
                PublicIncludePaths.Add(ModuleDirectory);

                // Engine-independent core lives at <repo>/core, outside the module. GetFullPath
                // normalises the "../.." so the /I path is clean on Windows.
                PublicIncludePaths.Add(System.IO.Path.GetFullPath(System.IO.Path.Combine(ModuleDirectory, "../../core")));

                // core/circuit/StadiumCircuit.cpp validates its arguments with `throw` (and the Pi unit
                // tests rely on that). UE builds with C++ exceptions off by default (C4530), so enable
                // them for this module only instead of forking the core error path.
                bEnableExceptions = true;
                bUseUnity = false;

                PublicDependencyModuleNames.AddRange(new string[]
                {
                        "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput"
                });

                // Needed later for procedural road meshes (Phase 1) and HUD (Phase 2).
                PrivateDependencyModuleNames.AddRange(new string[]
                {
                        "ProceduralMeshComponent", "UMG", "Slate", "SlateCore"
                });
        }
}
