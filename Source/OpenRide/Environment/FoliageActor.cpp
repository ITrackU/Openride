#include "Environment/FoliageActor.h"

#include "Circuit/CircuitActor.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "../../../core/foliage/FoliageScatter.h"

namespace
{
constexpr double MetersToUU = 100.0;
// Centre-line samples used for the clearance test; 128 is plenty for a 400 m loop.
constexpr int32 RoadSamples = 128;
}

AFoliageActor::AFoliageActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Instances = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("Instances"));
	SetRootComponent(Instances);
	// Grass must never block the rider or cast expensive shadows.
	Instances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Instances->SetCastShadow(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone"));
	if (ConeMesh.Succeeded())
	{
		FoliageMesh = ConeMesh.Object;
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GridMaterial(TEXT("/Engine/EngineMaterials/WorldGridMaterial"));
	if (GridMaterial.Succeeded())
	{
		FoliageMaterial = GridMaterial.Object;
	}
}

void AFoliageActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RebuildFoliage();
}

int32 AFoliageActor::RebuildFoliage()
{
	Instances->ClearInstances();
	if (!FoliageMesh)
	{
		return 0;
	}
	Instances->SetStaticMesh(FoliageMesh);
	if (FoliageMaterial)
	{
		Instances->SetMaterial(0, FoliageMaterial);
	}
	Instances->SetCullDistances(0, CullDistanceMeters > 0.f ? static_cast<int32>(CullDistanceMeters * MetersToUU) : 0);

	OpenRideCore::FFoliageParams Params;
	Params.DensityPerSqm = DensityPerSqm;
	Params.MaxInstances = MaxInstances;
	Params.RoadClearance = RoadClearanceMeters;
	Params.MinScale = MinScale;
	Params.MaxScale = FMath::Max(MinScale, MaxScale);
	Params.Seed = static_cast<uint32>(Seed);

	// Work in world metres; instances are added in world space below, so the actor's own
	// transform never skews the layout.
	std::vector<OpenRideCore::FPoint2> Road;
	double GroundZ = GetActorLocation().Z;
	if (Circuit)
	{
		FVector Origin, Extent;
		Circuit->GetActorBounds(false, Origin, Extent);
		Params.CenterX = Origin.X / MetersToUU;
		Params.CenterY = Origin.Y / MetersToUU;
		Params.HalfExtentX = Extent.X / MetersToUU + MarginMeters;
		Params.HalfExtentY = Extent.Y / MetersToUU + MarginMeters;
		GroundZ = Circuit->GetActorLocation().Z;

		const float Length = Circuit->GetLoopLengthMeters();
		Road.reserve(RoadSamples);
		for (int32 I = 0; I < RoadSamples; ++I)
		{
			const FVector P = Circuit->GetTransformAtDistance(Length * I / RoadSamples).GetLocation();
			Road.push_back({P.X / MetersToUU, P.Y / MetersToUU});
		}
	}
	else
	{
		const FVector Loc = GetActorLocation();
		Params.CenterX = Loc.X / MetersToUU;
		Params.CenterY = Loc.Y / MetersToUU;
		Params.HalfExtentX = ManualHalfExtentMeters.X;
		Params.HalfExtentY = ManualHalfExtentMeters.Y;
	}

	const std::vector<OpenRideCore::FFoliageInstance> Scatter = OpenRideCore::ScatterFoliage(Params, Road);

	TArray<FTransform> Transforms;
	Transforms.Reserve(static_cast<int32>(Scatter.size()));
	for (const OpenRideCore::FFoliageInstance& S : Scatter)
	{
		Transforms.Add(FTransform(FRotator(0.f, S.YawDegrees, 0.f),
		                          FVector(S.X * MetersToUU, S.Y * MetersToUU, GroundZ),
		                          FVector(S.Scale)));
	}
	// bWorldSpace = true: see comment above.
	Instances->AddInstances(Transforms, /*bShouldReturnIndices=*/false, /*bWorldSpace=*/true);
	return Transforms.Num();
}
