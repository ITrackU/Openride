#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "OpenRideHUD.generated.h"

/**
 * Minimal viewer-mode HUD: speed (km/h), distance, lap counter, lap time.
 * Uses AHUD canvas drawing (no UMG assets) so it works without any .uasset.
 */
UCLASS()
class OPENRIDE_API AOpenRideHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	/** Text scale; raise for high-DPI screens. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD", meta = (ClampMin = "0.5"))
	float TextScale = 1.5f;

	/** Formats seconds as M:SS.mmm; negative values give "--:--.---". Static so it is easy to test. */
	static FString FormatTime(float Seconds);
};
