#include "Circuit/CircuitActor.h"

#include "Components/SplineComponent.h"
#include "../../../core/circuit/StadiumCircuit.h"

namespace
{
// Core math is in metres, Unreal works in centimetres.
constexpr double MetersToUU = 100.0;
}

ACircuitActor::ACircuitActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	SetRootComponent(Spline);
	Spline->SetClosedLoop(true);
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
