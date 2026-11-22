#include "CyclistAnimInstance.h"

#include "GameFramework/Actor.h"

void UCyclistAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Model = OpenRideCore::FCyclistAnimState();
	bHasPreviousYaw = false;
	bHasPreviousLocation = false;
}

void UCyclistAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	const AActor* Owner = GetOwningActor();
	if (!Owner || DeltaSeconds <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	// Speed is measured from position change: the pawn is teleported along the spline with
	// SetActorLocation, so AActor::GetVelocity() would always be zero. Projected on forward so
	// sideways slides do not count; cm -> m.
	const FVector Location = Owner->GetActorLocation();
	float ForwardSpeedMps = 0.f;
	if (bHasPreviousLocation)
	{
		ForwardSpeedMps = FVector::DotProduct(Location - PreviousLocation, Owner->GetActorForwardVector()) / 100.f / DeltaSeconds;
	}
	PreviousLocation = Location;
	bHasPreviousLocation = true;

	// Yaw rate from frame-to-frame rotation (UE: positive yaw = turning right).
	const float YawDegrees = Owner->GetActorRotation().Yaw;
	float YawRateRad = 0.f;
	if (bHasPreviousYaw)
	{
		const float DeltaYaw = FMath::FindDeltaAngleDegrees(PreviousYawDegrees, YawDegrees);
		YawRateRad = FMath::DegreesToRadians(DeltaYaw) / DeltaSeconds;
	}
	PreviousYawDegrees = YawDegrees;
	bHasPreviousYaw = true;

	OpenRideCore::FCyclistAnimParams Params;
	Params.GearDevelopment = GearDevelopmentMeters;
	Params.MaxCadence = MaxCadenceRpm;
	Params.SprintOnCadence = SprintCadenceRpm;
	Params.SprintOffCadence = SprintCadenceRpm - 10.0;
	Params.MaxLeanRadians = FMath::DegreesToRadians(MaxLeanDegrees);

	OpenRideCore::StepAnimModel(Model, Params, ForwardSpeedMps, YawRateRad, DeltaSeconds);

	Speed = static_cast<float>(Model.Speed);
	Cadence = static_cast<float>(Model.Cadence);
	Lean = static_cast<float>(Model.Lean);
	bSprinting = Model.bSprinting;
	bPedaling = Model.bPedaling;
}
