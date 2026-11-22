#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FoliageActor.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;
class ACircuitActor;

/**
 * Scatters grass tufts around the circuit using one hierarchical instanced static mesh
 * (cheap to draw, culls per cluster). Placement comes from core/foliage; density is a
 * simple instances-per-square-metre setting so it can be tuned for weak GPUs.
 */
UCLASS()
class OPENRIDE_API AFoliageActor : public AActor
{
	GENERATED_BODY()

public:
	AFoliageActor();

	/** Circuit to keep clear (road + verge) and to fit the scatter area around. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
	TObjectPtr<ACircuitActor> Circuit;

	/** Mesh to instance; defaults to an engine cone as a stand-in for a grass tuft. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
	TObjectPtr<UStaticMesh> FoliageMesh;

	/** Optional override material (placeholder grid material otherwise). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage")
	TObjectPtr<UMaterialInterface> FoliageMaterial;

	/** Instances per square metre (before the road clearance cull). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Foliage|Density", meta = (ClampMin = "0.0", ClampMax = "20.0"))
	float DensityPerSqm = 1.f;

	/** Hard cap on instance count; the cell size grows to respect it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Foliage|Density", meta = (ClampMin = "0", ClampMax = "1000000"))
	int32 MaxInstances = 100000;

	/** Free strip kept around the road centre-line, in metres (half road width + verge). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Foliage|Placement", meta = (ClampMin = "0.0"))
	float RoadClearanceMeters = 6.f;

	/** Margin around the circuit bounds that gets foliage, in metres. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Foliage|Placement", meta = (ClampMin = "0.0"))
	float MarginMeters = 60.f;

	/** Half-size (X, Y) in metres used when no Circuit is assigned. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Foliage|Placement", meta = (ClampMin = "10.0"))
	FVector2D ManualHalfExtentMeters = FVector2D(120.f, 90.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Foliage|Placement", meta = (ClampMin = "0.01"))
	float MinScale = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Foliage|Placement", meta = (ClampMin = "0.01"))
	float MaxScale = 1.3f;

	/** Same seed = same layout. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Foliage|Placement")
	int32 Seed = 1;

	/** Distance in metres beyond which instances are culled (0 = never). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Foliage|Performance", meta = (ClampMin = "0.0"))
	float CullDistanceMeters = 150.f;

	/** Clears and regenerates all instances. Returns the instance count. */
	UFUNCTION(BlueprintCallable, Category = "Foliage")
	int32 RebuildFoliage();

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foliage")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> Instances;
};
