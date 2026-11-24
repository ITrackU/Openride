#include "Environment/BuildingPlacerActor.h"

#include "Circuit/CircuitActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Environment/BuildingSetData.h"
#include "Materials/MaterialInterface.h"
#include "../../../core/buildings/BuildingPlacer.h"

namespace
{
constexpr double MetersToUU = 100.0;
constexpr int32 RoadSamples = 128;
}

ABuildingPlacerActor::ABuildingPlacerActor()
{
	PrimaryActorTick.bCanEverTick = false;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void ABuildingPlacerActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RebuildBuildings();
}

int32 ABuildingPlacerActor::RebuildBuildings()
{
	for (UInstancedStaticMeshComponent* Comp : VariantComponents)
	{
		if (Comp)
		{
			Comp->DestroyComponent();
		}
	}
	VariantComponents.Reset();

	if (!Circuit)
	{
		return 0;
	}

	const UBuildingSetData* Set = BuildingSet;
	if (!Set || Set->Variants.Num() == 0)
	{
		if (!DefaultSet)
		{
			DefaultSet = NewObject<UBuildingSetData>(this, NAME_None, RF_Transient);
			DefaultSet->FillDefaults();
		}
		Set = DefaultSet;
	}

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UMaterialInterface* GridMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/WorldGridMaterial.WorldGridMaterial"));

	std::vector<OpenRideCore::FBuildingVariantSpec> Specs;
	for (const FBuildingVariant& V : Set->Variants)
	{
		OpenRideCore::FBuildingVariantSpec S;
		S.MinWidth = V.WidthRangeMeters.X;  S.MaxWidth = FMath::Max(V.WidthRangeMeters.X, V.WidthRangeMeters.Y);
		S.MinDepth = V.DepthRangeMeters.X;  S.MaxDepth = FMath::Max(V.DepthRangeMeters.X, V.DepthRangeMeters.Y);
		S.MinHeight = V.HeightRangeMeters.X; S.MaxHeight = FMath::Max(V.HeightRangeMeters.X, V.HeightRangeMeters.Y);
		S.Weight = V.Weight;
		Specs.push_back(S);
	}

	OpenRideCore::FBuildingParams Params;
	Params.Spacing = SpacingMeters;
	Params.Setback = SetbackMeters;
	Params.SetbackJitter = SetbackJitterMeters;
	Params.Gap = GapMeters;
	Params.Seed = static_cast<uint32>(Seed);
	Params.MaxBuildings = MaxBuildings;

	std::vector<OpenRideCore::FPoint2> Road;
	const float Length = Circuit->GetLoopLengthMeters();
	Road.reserve(RoadSamples);
	for (int32 I = 0; I < RoadSamples; ++I)
	{
		const FVector P = Circuit->GetTransformAtDistance(Length * I / RoadSamples).GetLocation();
		Road.push_back({P.X / MetersToUU, P.Y / MetersToUU});
	}
	const double GroundZ = Circuit->GetActorLocation().Z;

	const std::vector<OpenRideCore::FBuildingPlacement> Placements = OpenRideCore::PlaceBuildings(Params, Specs, Road);

	// One component per variant that was actually used.
	TArray<UInstancedStaticMeshComponent*> Comps;
	Comps.Init(nullptr, Set->Variants.Num());

	for (const OpenRideCore::FBuildingPlacement& B : Placements)
	{
		const FBuildingVariant& V = Set->Variants[B.VariantIndex];
		UStaticMesh* Mesh = V.Mesh ? V.Mesh.Get() : CubeMesh;
		if (!Mesh)
		{
			continue;
		}
		UInstancedStaticMeshComponent*& Comp = Comps[B.VariantIndex];
		if (!Comp)
		{
			Comp = NewObject<UInstancedStaticMeshComponent>(this, NAME_None, RF_Transient);
			Comp->SetStaticMesh(Mesh);
			UMaterialInterface* Mat = V.Material ? V.Material.Get() : GridMaterial;
			if (Mat)
			{
				Comp->SetMaterial(0, Mat);
			}
			Comp->SetupAttachment(GetRootComponent());
			Comp->RegisterComponent();
			VariantComponents.Add(Comp);
		}

		// Stretch the mesh to the footprint; lift by its own bounds so any pivot sits on the ground.
		const FBoxSphereBounds MeshBounds = Mesh->GetBounds();
		const FVector Size = MeshBounds.BoxExtent * 2.0;
		const FVector Scale(B.Width * MetersToUU / FMath::Max(Size.X, 1.0),
		                    B.Depth * MetersToUU / FMath::Max(Size.Y, 1.0),
		                    B.Height * MetersToUU / FMath::Max(Size.Z, 1.0));
		const double BaseOffset = -(MeshBounds.Origin.Z - MeshBounds.BoxExtent.Z) * Scale.Z;
		Comp->AddInstance(FTransform(FRotator(0.f, B.YawDegrees, 0.f),
		                             FVector(B.X * MetersToUU, B.Y * MetersToUU, GroundZ + BaseOffset),
		                             Scale),
		                  /*bWorldSpace=*/true);
	}
	return static_cast<int32>(Placements.size());
}
