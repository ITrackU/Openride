#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../../core/race/RaceModel.h"
#include "RaceManager.generated.h"

class ACircuitActor;
class ACyclistCharacter;
class ABikeActor;
class ANPCCyclist;

/**
 * Spawns N NPC cyclists with randomised look, speed, lane and start offset, then keeps them from
 * riding through each other: every tick each NPC gets a lane/speed target from the engine-independent
 * core/race model (overtake via a free lane, or follow the rider ahead if boxed in).
 * Place one in the level; set NPCClass to the Blueprint that has the rider mesh + ABP assigned.
 */
UCLASS()
class OPENRIDE_API ARaceManager : public AActor
{
	GENERATED_BODY()

public:
	ARaceManager();

	/** Circuit to ride on; auto-detected (first ACircuitActor in the level) when null. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	TObjectPtr<ACircuitActor> Circuit;

	/** NPC Blueprint to spawn; falls back to the C++ ANPCCyclist (no rider mesh) when null. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	TSubclassOf<ANPCCyclist> NPCClass;

	/** Bike Blueprint handed to every NPC (null = NPC default). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	TSubclassOf<ABikeActor> BikeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race", meta = (ClampMin = "0", ClampMax = "50"))
	int32 NumNPCs = 8;

	/** Same seed = same field (speeds, lanes, colours, start spots). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 Seed = 1;

	/** Slowest / fastest NPC target speed in m/s. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race", meta = (ClampMin = "0.0"))
	float MinSpeedMps = 7.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race", meta = (ClampMin = "0.0"))
	float MaxSpeedMps = 11.f;

	/** Turns avoidance off (NPCs then just hold their own lane and speed). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race|Avoidance")
	bool bEnableAvoidance = true;

	/** How far ahead (m) an NPC looks for slower riders. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race|Avoidance", meta = (ClampMin = "1.0"))
	float LookAheadMeters = 12.f;

	/** Sideways gap (m) needed to pass someone. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race|Avoidance", meta = (ClampMin = "0.3"))
	float LaneClearanceMeters = 1.f;

	/** Kept clear of the road edge (m) so riders stay on the asphalt. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race|Avoidance", meta = (ClampMin = "0.0"))
	float EdgeMarginMeters = 0.8f;

	UFUNCTION(BlueprintPure, Category = "Race")
	const TArray<ANPCCyclist*>& GetNPCs() const { return NPCs; }

	/** Destroys existing NPCs and spawns a new field (also runs at BeginPlay). */
	UFUNCTION(BlueprintCallable, Category = "Race")
	void SpawnNPCs();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Transient)
	TArray<TObjectPtr<ANPCCyclist>> NPCs;

private:
	/** Each NPC's own choice (what it wants when the road is clear), parallel to NPCs. */
	TArray<OpenRideCore::FRaceSpawnSlot> Slots;

	OpenRideCore::FAvoidanceParams MakeAvoidanceParams() const;
};
