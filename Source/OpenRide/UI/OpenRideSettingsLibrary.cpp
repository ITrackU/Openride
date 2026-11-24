#include "OpenRideSettingsLibrary.h"

#include "GameFramework/GameUserSettings.h"
#include "Engine/Engine.h"
#include "../../../core/settings/SettingsModel.h"

namespace
{
	UGameUserSettings* GetSettings()
	{
		return GEngine ? GEngine->GetGameUserSettings() : nullptr;
	}
}

TArray<FIntPoint> UOpenRideSettingsLibrary::GetOfferedResolutions()
{
	TArray<FIntPoint> Out;
	for (const OpenRideCore::FResolution& R : OpenRideCore::DefaultResolutions())
	{
		Out.Add(FIntPoint(R.Width, R.Height));
	}
	return Out;
}

int32 UOpenRideSettingsLibrary::GetCurrentResolutionIndex()
{
	const UGameUserSettings* Settings = GetSettings();
	if (!Settings)
	{
		return -1;
	}
	const FIntPoint Cur = Settings->GetScreenResolution();
	return OpenRideCore::NearestResolutionIndex(OpenRideCore::DefaultResolutions(), { Cur.X, Cur.Y });
}

void UOpenRideSettingsLibrary::ApplyResolution(FIntPoint Resolution)
{
	if (UGameUserSettings* Settings = GetSettings())
	{
		Settings->SetScreenResolution(Resolution);
		// Keeps the current window mode; bCheckForCommandLineOverrides = false so menu wins.
		Settings->ApplyResolutionSettings(false);
	}
}

void UOpenRideSettingsLibrary::ApplyQualityLevel(int32 Level)
{
	if (UGameUserSettings* Settings = GetSettings())
	{
		// SetOverallScalabilityLevel sets all sg.* groups (view distance, shadows, ...) at once.
		Settings->SetOverallScalabilityLevel(OpenRideCore::ClampQuality(Level));
		Settings->ApplySettings(false);
	}
}

int32 UOpenRideSettingsLibrary::GetQualityLevel()
{
	const UGameUserSettings* Settings = GetSettings();
	return Settings ? Settings->GetOverallScalabilityLevel() : -1;
}

FString UOpenRideSettingsLibrary::GetQualityName(int32 Level)
{
	return FString(OpenRideCore::QualityName(
		static_cast<OpenRideCore::EQualityLevel>(OpenRideCore::ClampQuality(Level))));
}

void UOpenRideSettingsLibrary::SaveSettings()
{
	if (UGameUserSettings* Settings = GetSettings())
	{
		Settings->SaveSettings();
	}
}

void UOpenRideSettingsLibrary::LoadAndApplySavedSettings()
{
	if (UGameUserSettings* Settings = GetSettings())
	{
		Settings->LoadSettings();
		Settings->ApplySettings(false);
	}
}
