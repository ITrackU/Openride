using UnrealBuildTool;

public class OpenRide : ModuleRules
{
        public OpenRide(ReadOnlyTargetRules Target) : base(Target)
        {
                PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

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
