using UnrealBuildTool;
using System.Collections.Generic;

public class OpenRideEditorTarget : TargetRules
{
	public OpenRideEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("OpenRide");
	}
}
