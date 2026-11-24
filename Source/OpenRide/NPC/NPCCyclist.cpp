#include "NPC/NPCCyclist.h"

#include "Circuit/CircuitActor.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Cyclist/BikeActor.h"
#include "Cyclist/CyclistAppearanceData.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ANPCCyclist::ANPCCyclist()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	RiderMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RiderMesh"));
	RiderMesh->SetupAttachment(Root);
	RiderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// ABP_Cyclist reads the model from this actor, so bones must update every frame.
	RiderMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
}

void ANPCCyclist::BeginPlay()
{
	Super::BeginPlay();

	if (!Circuit)
	{
		Circuit = Cast<ACircuitActor>(UGameplayStatics::GetActorOfClass(this, ACircuitActor::StaticClass()));
	}

	State.Distance = StartDistanceMeters;
	State.Lane = LaneOffsetMeters;
	// Start at cruising speed: NPCs placed at race start should not all begin from standstill.
	State.Speed = TargetSpeedMps;

	// Same bike class as the player; fall back to the C++ base so a missing Blueprint does not crash.
	UWorld* World = GetWorld();
	const TSubclassOf<ABikeActor> ClassToSpawn = BikeClass ? BikeClass : TSubclassOf<ABikeActor>(ABikeActor::StaticClass());
	if (World)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Bike = World->SpawnActor<ABikeActor>(ClassToSpawn, GetActorTransform(), Params);
	}
	if (Bike)
	{
		Bike->AttachToActor(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		Bike->AttachRider(RiderMesh);
		if (Appearance)
		{
			Appearance->ApplyToRider(RiderMesh);
			Appearance->ApplyToBike(Bike);
		}
	}

	UpdateTransform();
}

void ANPCCyclist::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	OpenRideCore::FNpcFollowerParams Params;
	Params.TargetSpeed = TargetSpeedMps;
	Params.Acceleration = AccelerationMps2;
	Params.Deceleration = DecelerationMps2;
	Params.TargetLane = LaneOffsetMeters;
	Params.LateralSpeed = LateralSpeedMps;
	OpenRideCore::StepNpcFollower(State, Params, DeltaSeconds);

	UpdateTransform();
}

void ANPCCyclist::UpdateTransform()
{
	if (!Circuit)
	{
		return;
	}

	// Circuit wraps the distance (closed loop). The lane offset is applied along the track's
	// right vector so it stays correct through the bends. Metres -> cm for UE.
	const FTransform OnTrack = Circuit->GetTransformAtDistance(static_cast<float>(State.Distance));
	const FVector Location = OnTrack.GetLocation() + OnTrack.GetRotation().GetRightVector() * static_cast<float>(State.Lane) * 100.f;
	SetActorLocationAndRotation(Location, OnTrack.GetRotation());
}
