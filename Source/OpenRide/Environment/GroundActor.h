#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GroundActor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UMaterialInterface;
class ACircuitActor;

/**
 * Flat grass ground plane placed just under the road, plus a box marking the playable
 * bounds. Sized from the circuit's bounding box plus a margin, or from manual extents.
 */
UCLASS()
class OPENRIDE_API AGroundActor : public AActor
{
	GENERATED_BODY()

public:
	AGroundActor();

	/** If set, the ground is fitted around this circuit (bounding box + margin). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
	TObjectPtr<ACircuitActor> Circuit;

	/** Free space kept around the road on every side, in metres. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "0.0"))
	float MarginMeters = 60.f;

	/** Half-size (X, Y) in metres used when no Circuit is assigned. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "10.0"))
	FVector2D ManualHalfExtentMeters = FVector2D(120.f, 90.f);

	/** How far below the road surface the ground sits, in centimetres (avoids z-fighting). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "0.5"))
	float DepthBelowRoadCm = 5.f;

	/** Height of the playable-bounds box in metres. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "1.0"))
	float BoundsHeightMeters = 50.f;

	/** Grass material; falls back to the engine grid material until a real one is assigned. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
	TObjectPtr<UMaterialInterface> GrassMaterial;

	/** Refits the plane and bounds box. */
	UFUNCTION(BlueprintCallable, Category = "Ground")
	void RebuildGround();

	/** True if a world location lies inside the playable bounds (XY only). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ground")
	bool IsInsideBounds(const FVector& WorldLocation) const;

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground")
	TObjectPtr<UStaticMeshComponent> GroundMesh;

	/** Query-only box describing the playable area; no collision response. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground")
	TObjectPtr<UBoxComponent> PlayableBounds;
};
