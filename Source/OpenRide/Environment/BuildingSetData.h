#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BuildingSetData.generated.h"

class UStaticMesh;
class UMaterialInterface;

/** One kind of building: a mesh plus the size range it is stretched to. */
USTRUCT(BlueprintType)
struct FBuildingVariant
{
	GENERATED_BODY()

	/** Mesh to instance; empty = engine cube placeholder. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	TObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
	TObjectPtr<UMaterialInterface> Material;

	/** Footprint width along the road, in metres. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building", meta = (ClampMin = "1.0"))
	FVector2D WidthRangeMeters = FVector2D(8.f, 12.f);

	/** Footprint depth away from the road, in metres. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building", meta = (ClampMin = "1.0"))
	FVector2D DepthRangeMeters = FVector2D(8.f, 10.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building", meta = (ClampMin = "1.0"))
	FVector2D HeightRangeMeters = FVector2D(6.f, 9.f);

	/** Relative probability of picking this variant. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building", meta = (ClampMin = "0.0"))
	float Weight = 1.f;
};

/** Data asset listing the building variants a placer may use (houses, blocks, towers...). */
UCLASS(BlueprintType)
class OPENRIDE_API UBuildingSetData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buildings")
	TArray<FBuildingVariant> Variants;

	/** Three placeholder variants (small house, house, block) so the level works without assets. */
	void FillDefaults();
};
