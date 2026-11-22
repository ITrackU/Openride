#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SkyLightingActor.generated.h"

class UDirectionalLightComponent;
class USkyAtmosphereComponent;
class USkyLightComponent;
class UExponentialHeightFogComponent;
class UPostProcessComponent;

/**
 * One-stop sky and lighting rig: sun (directional light), sky atmosphere, sky light, height fog
 * and an unbound post-process (auto exposure). A single TimeOfDayHours value drives the sun
 * position, colour and ambient level through core/sky, so day/night is one property.
 */
UCLASS()
class OPENRIDE_API ASkyLightingActor : public AActor
{
	GENERATED_BODY()

public:
	ASkyLightingActor();

	/** Clock time 0..24; 6 = sunrise (east), 12 = noon, 18 = sunset (west). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float TimeOfDayHours = 10.f;

	/** Highest sun elevation reached at noon, in degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sky", meta = (ClampMin = "10.0", ClampMax = "90.0"))
	float MaxSunElevationDeg = 60.f;

	/** If > 0 the clock advances by this many in-game hours per real second (0 = frozen). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky", meta = (ClampMin = "0.0"))
	float TimeSpeedHoursPerSecond = 0.f;

	/** Sets the clock (wrapped into 0..24) and refreshes the lighting. */
	UFUNCTION(BlueprintCallable, Category = "Sky")
	void SetTimeOfDay(float Hours);

	/** Re-applies TimeOfDayHours to every component. */
	UFUNCTION(BlueprintCallable, Category = "Sky")
	void UpdateSky();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sky")
	TObjectPtr<UDirectionalLightComponent> Sun;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sky")
	TObjectPtr<USkyAtmosphereComponent> Atmosphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sky")
	TObjectPtr<USkyLightComponent> SkyLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sky")
	TObjectPtr<UExponentialHeightFogComponent> Fog;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sky")
	TObjectPtr<UPostProcessComponent> PostProcess;
};
