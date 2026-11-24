#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildingPlacerActor.generated.h"

class UInstancedStaticMeshComponent;
class UBuildingSetData;
class ACircuitActor;

/**
 * Places houses/buildings around the outside of the circuit, driven by a UBuildingSetData
 * asset (variants with meshes and size ranges). Placement comes from core/buildings; one
 * instanced mesh component is created per variant.
 */
UCLASS()
class OPENRIDE_API ABuildingPlacerActor : public AActor
{
	GENERATED_BODY()

public:
	ABuildingPlacerActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buildings")
	TObjectPtr<ACircuitActor> Circuit;

	/** Variants to use; if empty, built-in placeholder variants are used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buildings")
	TObjectPtr<UBuildingSetData> BuildingSet;

	/** Distance along the road between placement attempts, in metres. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buildings|Placement", meta = (ClampMin = "2.0"))
	float SpacingMeters = 16.f;

	/** Minimum gap between the road centre-line and any building, in metres. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buildings|Placement", meta = (ClampMin = "0.0"))
	float SetbackMeters = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buildings|Placement", meta = (ClampMin = "0.0"))
	float SetbackJitterMeters = 6.f;

	/** Minimum clearance between neighbouring buildings, in metres. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buildings|Placement", meta = (ClampMin = "0.0"))
	float GapMeters = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buildings|Placement")
	int32 Seed = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buildings|Placement", meta = (ClampMin = "0", ClampMax = "5000"))
	int32 MaxBuildings = 500;

	/** Clears and regenerates all buildings. Returns how many were placed. */
	UFUNCTION(BlueprintCallable, Category = "Buildings")
	int32 RebuildBuildings();

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	/** One component per variant, recreated on every rebuild. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> VariantComponents;

	UPROPERTY(Transient)
	TObjectPtr<UBuildingSetData> DefaultSet;
};
