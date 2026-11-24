#include "BikeActor.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"

const FName ABikeActor::SaddleSocket(TEXT("Saddle"));
const FName ABikeActor::HandLeftSocket(TEXT("HandLeft"));
const FName ABikeActor::HandRightSocket(TEXT("HandRight"));
const FName ABikeActor::FootLeftSocket(TEXT("FootLeft"));
const FName ABikeActor::FootRightSocket(TEXT("FootRight"));

ABikeActor::ABikeActor()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	LeanPivot = CreateDefaultSubobject<USceneComponent>(TEXT("LeanPivot"));
	LeanPivot->SetupAttachment(Root);

	BikeMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BikeMesh"));
	BikeMesh->SetupAttachment(LeanPivot);
	BikeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// The anim instance reads the model from this actor, so bones must update every frame.
	BikeMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
}

void ABikeActor::AttachRider(USkeletalMeshComponent* RiderMesh)
{
	if (!RiderMesh || !BikeMesh)
	{
		return;
	}
	// Rider rest pose is already seated (pipeline doc), so no offset is needed: snap to the socket.
	RiderMesh->AttachToComponent(BikeMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SaddleSocket);
}

FVector ABikeActor::GetSocketWorldLocation(FName SocketName) const
{
	return BikeMesh ? BikeMesh->GetSocketLocation(SocketName) : GetActorLocation();
}

void ABikeActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (DeltaSeconds <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	// Measured from position/yaw change because pawns are teleported along the spline
	// (GetVelocity() would be zero). Forward component only; cm -> m.
	const FVector Location = GetActorLocation();
	const float Yaw = GetActorRotation().Yaw;
	float YawRateRad = 0.f;
	if (bHasPrevious)
	{
		MeasuredSpeedMps = FVector::DotProduct(Location - PreviousLocation, GetActorForwardVector()) / 100.f / DeltaSeconds;
		YawRateRad = FMath::DegreesToRadians(FMath::FindDeltaAngleDegrees(PreviousYawDegrees, Yaw)) / DeltaSeconds;
	}
	PreviousLocation = Location;
	PreviousYawDegrees = Yaw;
	bHasPrevious = true;

	OpenRideCore::FBikeParams Params;
	Params.WheelRadius = WheelRadiusCm / 100.0;
	Params.Wheelbase = WheelbaseCm / 100.0;
	Params.MaxLeanRadians = FMath::DegreesToRadians(MaxLeanDegrees);
	Params.MaxSteerRadians = FMath::DegreesToRadians(MaxSteerDegrees);
	OpenRideCore::StepBikeModel(State, Params, MeasuredSpeedMps, YawRateRad, DeltaSeconds);

	// Unreal roll is about +X (forward); positive = right side down, matching Lean's sign.
	LeanPivot->SetRelativeRotation(FRotator(0.f, 0.f, GetLeanDegrees()));
}
