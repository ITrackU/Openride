#include "Environment/GroundActor.h"

#include "Circuit/CircuitActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
constexpr double MetersToUU = 100.0;
// /Engine/BasicShapes/Plane is 100 x 100 uu.
constexpr double BasicPlaneSizeUU = 100.0;
}

AGroundActor::AGroundActor()
{
	PrimaryActorTick.bCanEverTick = false;

	GroundMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GroundMesh"));
	SetRootComponent(GroundMesh);
	GroundMesh->SetCastShadow(false);
	GroundMesh->SetCollisionProfileName(TEXT("BlockAll"));

	PlayableBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("PlayableBounds"));
	PlayableBounds->SetupAttachment(GroundMesh);
	PlayableBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane"));
	if (PlaneMesh.Succeeded())
	{
		GroundMesh->SetStaticMesh(PlaneMesh.Object);
	}

	// Placeholder until a real grass material exists (assets arrive on the PC side).
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GridMaterial(TEXT("/Engine/EngineMaterials/WorldGridMaterial"));
	if (GridMaterial.Succeeded())
	{
		GrassMaterial = GridMaterial.Object;
	}
}

void AGroundActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RebuildGround();
}

void AGroundActor::RebuildGround()
{
	FVector Center = GetActorLocation();
	FVector2D HalfExtentUU = ManualHalfExtentMeters * MetersToUU;

	if (Circuit)
	{
		FVector Origin, Extent;
		Circuit->GetActorBounds(false, Origin, Extent);
		Center = FVector(Origin.X, Origin.Y, Circuit->GetActorLocation().Z);
		HalfExtentUU = FVector2D(Extent.X, Extent.Y) + FVector2D(MarginMeters * MetersToUU);
	}

	// Sit slightly under the road so the asphalt always wins the depth test.
	SetActorLocation(FVector(Center.X, Center.Y, Center.Z - DepthBelowRoadCm));
	// Unit-scale plane is 100 uu wide, so full size / 100 gives the scale.
	GroundMesh->SetWorldScale3D(FVector(2.0 * HalfExtentUU.X / BasicPlaneSizeUU,
	                                    2.0 * HalfExtentUU.Y / BasicPlaneSizeUU, 1.0));

	// Box extent is set in world units on the (scaled) parent, so counter-scale it.
	const double HeightUU = BoundsHeightMeters * MetersToUU;
	PlayableBounds->SetWorldScale3D(FVector::OneVector);
	PlayableBounds->SetBoxExtent(FVector(HalfExtentUU.X, HalfExtentUU.Y, HeightUU * 0.5));
	PlayableBounds->SetWorldLocation(FVector(Center.X, Center.Y, Center.Z + HeightUU * 0.5));

	if (GrassMaterial)
	{
		GroundMesh->SetMaterial(0, GrassMaterial);
	}
}

bool AGroundActor::IsInsideBounds(const FVector& WorldLocation) const
{
	const FVector Local = PlayableBounds->GetComponentTransform().InverseTransformPosition(WorldLocation);
	const FVector Extent = PlayableBounds->GetUnscaledBoxExtent();
	return FMath::Abs(Local.X) <= Extent.X && FMath::Abs(Local.Y) <= Extent.Y;
}
