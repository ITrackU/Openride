#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OpenRideSettingsLibrary.generated.h"

/**
 * Thin UGameUserSettings-backed apply/save layer for the settings menu.
 * Stateless statics so any widget/HUD/Blueprint can call it. The choices come from
 * core/settings (resolution list, quality levels 0..3 = sg.* scalability levels).
 */
UCLASS()
class OPENRIDE_API UOpenRideSettingsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Resolutions offered by the menu (core/settings DefaultResolutions). */
	UFUNCTION(BlueprintPure, Category = "Settings")
	static TArray<FIntPoint> GetOfferedResolutions();

	/** Index into GetOfferedResolutions() closest to the current resolution (-1 if none). */
	UFUNCTION(BlueprintPure, Category = "Settings")
	static int32 GetCurrentResolutionIndex();

	/** Sets the resolution (windowed-fullscreen mode kept) and applies it; does not save. */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	static void ApplyResolution(FIntPoint Resolution);

	/** Overall quality 0..3 (Low..Epic); clamped. Sets every sg.* group and applies. */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	static void ApplyQualityLevel(int32 Level);

	/** Current overall quality 0..3, or -1 if the groups are mixed (custom). */
	UFUNCTION(BlueprintPure, Category = "Settings")
	static int32 GetQualityLevel();

	/** Display name for a quality level ("Low".."Epic"). */
	UFUNCTION(BlueprintPure, Category = "Settings")
	static FString GetQualityName(int32 Level);

	/** Persists to GameUserSettings.ini. */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	static void SaveSettings();

	/** Loads saved settings and applies them; call once at start-up (e.g. game mode BeginPlay). */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	static void LoadAndApplySavedSettings();
};
