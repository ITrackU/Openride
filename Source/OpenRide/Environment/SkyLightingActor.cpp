#include "Environment/SkyLightingActor.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "../../../core/sky/SkyModel.h"

ASkyLightingActor::ASkyLightingActor()
{
	PrimaryActorTick.bCanEverTick = true;
	// Only ticks when a time speed is set; see Tick().
	PrimaryActorTick.bStartWithTickEnabled = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Sun = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Sun"));
	Sun->SetupAttachment(Root);
	Sun->SetMobility(EComponentMobility::Movable);
	// Lets the sky atmosphere render the sun disc and scatter from this light.
	Sun->SetAtmosphereSunLight(true);
	Sun->SetAtmosphereSunLightIndex(0);

	Atmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("Atmosphere"));
	Atmosphere->SetupAttachment(Root);

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(Root);
	SkyLight->SetMobility(EComponentMobility::Movable);
	// Re-captures the atmosphere every frame it changes, so ambient light follows the sun.
	SkyLight->bRealTimeCapture = true;

	Fog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("Fog"));
	Fog->SetupAttachment(Root);
	Fog->SetFogDensity(0.005f);

	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
	PostProcess->SetupAttachment(Root);
	PostProcess->bUnbound = true;
	// Auto exposure with a bounded range so night is dark-but-readable, noon is not blown out.
	PostProcess->Settings.bOverride_AutoExposureMinBrightness = true;
	PostProcess->Settings.bOverride_AutoExposureMaxBrightness = true;
	PostProcess->Settings.AutoExposureMinBrightness = 0.5f;
	PostProcess->Settings.AutoExposureMaxBrightness = 4.f;
	PostProcess->Settings.bOverride_AutoExposureBias = true;
}

void ASkyLightingActor::SetTimeOfDay(float Hours)
{
	TimeOfDayHours = static_cast<float>(OpenRideCore::WrapHours(Hours));
	UpdateSky();
}

void ASkyLightingActor::UpdateSky()
{
	const OpenRideCore::FSkyState S = OpenRideCore::ComputeSky(TimeOfDayHours, MaxSunElevationDeg);

	// The light travels opposite to the direction pointing at the sun, hence the 180 degree yaw flip.
	const FRotator SunRotation(-static_cast<float>(S.SunElevationDeg), static_cast<float>(S.SunAzimuthDeg) + 180.f, 0.f);
	Sun->SetWorldRotation(SunRotation);
	Sun->SetIntensity(static_cast<float>(S.SunIntensity));
	Sun->SetTemperature(static_cast<float>(S.SunTemperatureK));
	Sun->SetUseTemperature(true);
	// Avoid a ghost sun below the horizon at night.
	Sun->SetVisibility(!S.bNight);

	SkyLight->SetIntensity(static_cast<float>(S.SkyLightIntensity));
	PostProcess->Settings.AutoExposureBias = static_cast<float>(S.ExposureBias);
	SkyLight->MarkRenderStateDirty();
}

void ASkyLightingActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateSky();
}

void ASkyLightingActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (TimeSpeedHoursPerSecond > 0.f)
	{
		SetTimeOfDay(TimeOfDayHours + TimeSpeedHoursPerSecond * DeltaSeconds);
	}
}
