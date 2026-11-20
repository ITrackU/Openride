using UnrealBuildTool;
using System.Collections.Generic;

public class OpenRideTarget : TargetRules
{
	public OpenRideTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("OpenRide");
	}
}
