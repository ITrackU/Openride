#include "Circuit/CircuitActor.h"

#include "Components/SplineComponent.h"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "../../../core/circuit/StadiumCircuit.h"
#include "../../../core/road/RoadMesh.h"

namespace
{
// Core math is in metres, Unreal works in centimetres.
constexpr double MetersToUU = 100.0;

// Markings sit 2 cm above the asphalt to avoid z-fighting.
constexpr double MarkingLiftMeters = 0.02;

// Copies engine-independent geometry into a procedural mesh section (metres -> cm).
void FillSection(UProceduralMeshComponent* Mesh, int32 Section, const OpenRideCore::FRoadMesh& Src,
                 UMaterialInterface* Material)
{
	Mesh->ClearAllMeshSections();
	if (Src.Vertices.empty())
	{
		return;
	}
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	Vertices.Reserve(Src.Vertices.size());
	for (const OpenRideCore::FRoadVertex& V : Src.Vertices)
	{
		Vertices.Add(FVector(V.X * MetersToUU, V.Y * MetersToUU, V.Z * MetersToUU));
		Normals.Add(FVector::UpVector);
		UVs.Add(FVector2D(V.U, V.V)); // V is in metres: material tiles once per metre
	}
	Triangles.Reserve(Src.Indices.size());
	for (uint32 Index : Src.Indices)
	{
		Triangles.Add(static_cast<int32>(Index));
	}
	Mesh->CreateMeshSection(Section, Vertices, Triangles, Normals, UVs, TArray<FColor>(), TArray<FProcMeshTangent>(), false);
	if (Material)
	{
		Mesh->SetMaterial(Section, Material);
	}
}
}

ACircuitActor::ACircuitActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	SetRootComponent(Spline);
	Spline->SetClosedLoop(true);

	RoadMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("RoadMesh"));
	RoadMesh->SetupAttachment(Spline);
	RoadMesh->bUseAsyncCooking = true;
	RoadMesh->SetCastShadow(false);

	MarkingMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("MarkingMesh"));
	MarkingMesh->SetupAttachment(Spline);
	MarkingMesh->SetCastShadow(false);

	// Placeholder until a real asphalt material exists (assets arrive on the PC side).
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GridMaterial(TEXT("/Engine/EngineMaterials/WorldGridMaterial"));
	if (GridMaterial.Succeeded())
	{
		RoadMaterial = GridMaterial.Object;
		MarkingMaterial = GridMaterial.Object;
	}
}

void ACircuitActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RebuildSpline();
}

void ACircuitActor::RebuildSpline()
{
	// Radius must leave room for straights: 2*pi*R < Length.
	const double Radius = FMath::Min<double>(RadiusMeters, LengthMeters / (2.0 * PI) * 0.9);
	const OpenRideCore::FStadiumCircuit Circuit(LengthMeters, Radius);
	const std::vector<OpenRideCore::FCircuitSample> Samples = Circuit.GenerateSamples(NumSplinePoints);

	Spline->ClearSplinePoints(false);
	for (const OpenRideCore::FCircuitSample& S : Samples)
	{
		Spline->AddSplinePoint(FVector(S.X * MetersToUU, S.Y * MetersToUU, 0.0), ESplineCoordinateSpace::Local, false);
	}
	Spline->SetClosedLoop(true, false);
	Spline->UpdateSpline();

	RebuildRoadMesh();
}

void ACircuitActor::RebuildRoadMesh()
{
	const double Radius = FMath::Min<double>(RadiusMeters, LengthMeters / (2.0 * PI) * 0.9);
	const OpenRideCore::FStadiumCircuit Circuit(LengthMeters, Radius);
	const double Step = MeshStepMeters;

	FillSection(RoadMesh, 0, OpenRideCore::BuildLoopRibbon(Circuit, 0.0, WidthMeters, 0.0, Step), RoadMaterial);

	// Two solid edge lines plus a dashed centre line (the road is a two-way lane).
	const double EdgeOffset = WidthMeters * 0.5 - EdgeLineInsetMeters - MarkingWidthMeters * 0.5;
	OpenRideCore::FRoadMesh Markings;
	if (EdgeOffset > 0.0)
	{
		Markings.Append(OpenRideCore::BuildLoopRibbon(Circuit, EdgeOffset, MarkingWidthMeters, MarkingLiftMeters, Step));
		Markings.Append(OpenRideCore::BuildLoopRibbon(Circuit, -EdgeOffset, MarkingWidthMeters, MarkingLiftMeters, Step));
	}
	Markings.Append(OpenRideCore::BuildDashedRibbon(Circuit, 0.0, MarkingWidthMeters, MarkingLiftMeters,
	                                               DashLengthMeters, DashGapMeters, Step));
	FillSection(MarkingMesh, 0, Markings, MarkingMaterial);
}

FTransform ACircuitActor::GetTransformAtDistance(float Meters) const
{
	const float LengthUU = Spline->GetSplineLength();
	if (LengthUU <= KINDA_SMALL_NUMBER)
	{
		return GetActorTransform();
	}
	// FMath::Fmod keeps the sign of the input, so fix up negatives to wrap backwards too.
	float DistUU = FMath::Fmod(Meters * static_cast<float>(MetersToUU), LengthUU);
	if (DistUU < 0.f)
	{
		DistUU += LengthUU;
	}
	return Spline->GetTransformAtDistanceAlongSpline(DistUU, ESplineCoordinateSpace::World);
}

float ACircuitActor::GetLoopLengthMeters() const
{
	return Spline->GetSplineLength() / static_cast<float>(MetersToUU);
}
