using UnrealBuildTool;

public class OpenRide : ModuleRules
{
        public OpenRide(ReadOnlyTargetRules Target) : base(Target)
        {
                PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

                // BuildSettingsVersion.V7 + EngineIncludeOrderVersion.Latest no longer add the
                // module root, so includes like "Circuit/CircuitActor.h" need it explicitly.
                PublicIncludePaths.Add(ModuleDirectory);

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
