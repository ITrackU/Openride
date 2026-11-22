#include "NPC/RaceManager.h"

#include "Circuit/CircuitActor.h"
#include "Cyclist/BikeActor.h"
#include "Cyclist/CyclistAppearanceData.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NPC/NPCCyclist.h"
#include "Player/CyclistCharacter.h"

ARaceManager::ARaceManager()
{
	PrimaryActorTick.bCanEverTick = true;
	// Run after the NPCs/player moved, so avoidance sees this frame's positions.
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
}

void ARaceManager::BeginPlay()
{
	Super::BeginPlay();
	SpawnNPCs();
}

OpenRideCore::FAvoidanceParams ARaceManager::MakeAvoidanceParams() const
{
	OpenRideCore::FAvoidanceParams P;
	P.LoopLength = Circuit ? Circuit->GetLoopLengthMeters() : 400.0;
	P.HalfRoadWidth = Circuit ? Circuit->WidthMeters * 0.5 : 3.0;
	P.EdgeMargin = EdgeMarginMeters;
	P.LookAhead = LookAheadMeters;
	P.LaneClearance = LaneClearanceMeters;
	return P;
}

void ARaceManager::SpawnNPCs()
{
	for (ANPCCyclist* Npc : NPCs)
	{
		if (Npc)
		{
			Npc->Destroy();
		}
	}
	NPCs.Reset();
	Slots.Reset();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	if (!Circuit)
	{
		Circuit = Cast<ACircuitActor>(UGameplayStatics::GetActorOfClass(this, ACircuitActor::StaticClass()));
	}
	if (!Circuit)
	{
		UE_LOG(LogTemp, Warning, TEXT("ARaceManager: no ACircuitActor in the level, not spawning NPCs."));
		return;
	}

	const OpenRideCore::FAvoidanceParams Av = MakeAvoidanceParams();
	OpenRideCore::FRaceSpawnParams Plan;
	Plan.Count = NumNPCs;
	Plan.Seed = static_cast<uint32>(Seed);
	Plan.MinSpeed = MinSpeedMps;
	Plan.MaxSpeed = FMath::Max(MinSpeedMps, MaxSpeedMps);
	Plan.LoopLength = Av.LoopLength;
	Plan.HalfRoadWidth = Av.HalfRoadWidth;
	Plan.EdgeMargin = Av.EdgeMargin;
	for (const OpenRideCore::FRaceSpawnSlot& Slot : OpenRideCore::PlanSpawns(Plan))
	{
		Slots.Add(Slot);
	}

	const TSubclassOf<ANPCCyclist> ClassToSpawn = NPCClass ? NPCClass : TSubclassOf<ANPCCyclist>(ANPCCyclist::StaticClass());
	for (const OpenRideCore::FRaceSpawnSlot& Slot : Slots)
	{
		// Deferred spawn: ANPCCyclist reads these properties in BeginPlay.
		ANPCCyclist* Npc = World->SpawnActorDeferred<ANPCCyclist>(ClassToSpawn, FTransform::Identity, this, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!Npc)
		{
			continue;
		}
		Npc->Circuit = Circuit;
		if (BikeClass)
		{
			Npc->BikeClass = BikeClass;
		}
		Npc->StartDistanceMeters = static_cast<float>(Slot.StartDistance);
		Npc->TargetSpeedMps = static_cast<float>(Slot.TargetSpeed);
		Npc->LaneOffsetMeters = static_cast<float>(Slot.PreferredLane);
		Npc->Appearance = UCyclistAppearanceData::MakeRandom(Npc, static_cast<int32>(Slot.AppearanceSeed));
		Npc->FinishSpawning(FTransform::Identity);
		NPCs.Add(Npc);
	}
}

void ARaceManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bEnableAvoidance || !Circuit || NPCs.Num() != Slots.Num())
	{
		return;
	}

	const OpenRideCore::FAvoidanceParams Av = MakeAvoidanceParams();

	// Snapshot of every rider (NPCs + the player, who has no lateral offset so counts as lane 0),
	// so decisions in this frame do not depend on update order.
	TArray<OpenRideCore::FRaceRider> Riders;
	Riders.Reserve(NPCs.Num() + 1);
	for (const ANPCCyclist* Npc : NPCs)
	{
		OpenRideCore::FRaceRider R;
		if (Npc)
		{
			R.Distance = Npc->GetDistanceMeters();
			R.Speed = Npc->GetSpeedMps();
			R.Lane = Npc->GetCurrentLaneMeters();
		}
		else
		{
			R.Distance = -1.0e9; // destroyed NPC: far away, effectively ignored
		}
		Riders.Add(R);
	}
	const ACyclistCharacter* Player = Cast<ACyclistCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (Player)
	{
		Riders.Add({Player->DistanceMeters, Player->SpeedMps, 0.0});
	}

	std::vector<OpenRideCore::FRaceRider> Others;
	Others.reserve(Riders.Num());
	for (int32 i = 0; i < NPCs.Num(); ++i)
	{
		ANPCCyclist* Npc = NPCs[i];
		if (!Npc)
		{
			continue;
		}
		Others.clear();
		for (int32 j = 0; j < Riders.Num(); ++j)
		{
			if (j != i)
			{
				Others.push_back(Riders[j]);
			}
		}
		const OpenRideCore::FRaceSpawnSlot& Slot = Slots[i];
		const OpenRideCore::FAvoidanceDecision D =
			OpenRideCore::DecideAvoidance(Riders[i], Slot.PreferredLane, Slot.TargetSpeed, Others, Av);
		Npc->LaneOffsetMeters = static_cast<float>(D.TargetLane);
		Npc->TargetSpeedMps = static_cast<float>(D.TargetSpeed);
	}
}
