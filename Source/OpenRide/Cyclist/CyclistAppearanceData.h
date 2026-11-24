#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CyclistAppearanceData.generated.h"

class ABikeActor;
class USkeletalMeshComponent;

/**
 * Per-cyclist look: every rider and bike shares the same base meshes (docs/CYCLIST_PIPELINE.md)
 * and only differs by these colours, applied through dynamic material instances.
 * Create assets in the editor (Miscellaneous > Data Asset) for hand-made looks, or use
 * MakeRandom() for NPCs. Randomisation maths lives in core/appearance (unit-tested).
 */
UCLASS(BlueprintType)
class OPENRIDE_API UCyclistAppearanceData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FLinearColor JerseyColor = FLinearColor(0.8f, 0.05f, 0.05f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FLinearColor ShortsColor = FLinearColor(0.02f, 0.02f, 0.02f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FLinearColor HelmetColor = FLinearColor(0.9f, 0.9f, 0.9f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FLinearColor FrameColor = FLinearColor(0.05f, 0.1f, 0.6f);

	/** Rider master-material vector parameters (names fixed by the pipeline doc). */
	static const FName JerseyParam;
	static const FName ShortsParam;
	static const FName HelmetParam;
	static const FName FrameParam;

	/** Creates (or reuses) dynamic material instances on every slot of the rider mesh and sets the rider colours. */
	UFUNCTION(BlueprintCallable, Category = "Appearance")
	void ApplyToRider(USkeletalMeshComponent* RiderMesh) const;

	/** Same for the bike mesh (frame colour). */
	UFUNCTION(BlueprintCallable, Category = "Appearance")
	void ApplyToBike(ABikeActor* Bike) const;

	/** Transient random appearance, reproducible for a given seed (used by the NPC spawner). */
	UFUNCTION(BlueprintCallable, Category = "Appearance", meta = (WorldContext = "Outer"))
	static UCyclistAppearanceData* MakeRandom(UObject* Outer, int32 Seed);
};
