#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../../core/npc/NpcFollower.h"
#include "NPCCyclist.generated.h"

class ABikeActor;
class ACircuitActor;
class UCyclistAppearanceData;
class USkeletalMeshComponent;

/**
 * AI rider: the same rider + bike classes as the player (docs/CYCLIST_PIPELINE.md), driven along
 * the circuit by distance at a configurable target speed with a lateral lane offset.
 * Assign the rider SkeletalMesh + ABP_Cyclist in a Blueprint subclass (like the player);
 * the bike is spawned at BeginPlay and measures its own motion, so no extra wiring is needed.
 * Speed/lane maths lives in core/npc (engine-independent, unit-tested).
 */
UCLASS()
class OPENRIDE_API ANPCCyclist : public AActor
{
	GENERATED_BODY()

public:
	ANPCCyclist();

	/** Circuit to follow; auto-detected (first ACircuitActor in the level) when null. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	TObjectPtr<ACircuitActor> Circuit;

	/** Bike class to spawn; set to the Blueprint that has SK_Bike and ABP_Bike assigned. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	TSubclassOf<ABikeActor> BikeClass;

	/** Optional look; when null the rider keeps the default materials. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	TObjectPtr<UCyclistAppearanceData> Appearance;

	/** Speed the NPC tries to hold, in m/s (9 = ~32 km/h). Can be changed at runtime. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement", meta = (ClampMin = "0.0"))
	float TargetSpeedMps = 9.f;

	/** Acceleration towards the target speed, in m/s^2. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement", meta = (ClampMin = "0.0"))
	float AccelerationMps2 = 1.5f;

	/** Deceleration towards a lower target speed, in m/s^2. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement", meta = (ClampMin = "0.0"))
	float DecelerationMps2 = 3.f;

	/** Lateral offset from the centre line in metres (positive = right of travel). Keep within half the road width. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
	float LaneOffsetMeters = 0.f;

	/** Max sideways drift in m/s, so lane changes are smooth. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement", meta = (ClampMin = "0.0"))
	float LateralSpeedMps = 0.8f;

	/** Distance along the circuit at spawn, in metres. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
	float StartDistanceMeters = 0.f;

	/** Current distance along the circuit in metres (wrapped by the circuit, grows unbounded here). */
	UFUNCTION(BlueprintPure, Category = "NPC")
	float GetDistanceMeters() const { return static_cast<float>(State.Distance); }

	UFUNCTION(BlueprintPure, Category = "NPC")
	float GetSpeedMps() const { return static_cast<float>(State.Speed); }

	/** Current (smoothed) lateral offset in metres. */
	UFUNCTION(BlueprintPure, Category = "NPC")
	float GetCurrentLaneMeters() const { return static_cast<float>(State.Lane); }

	UFUNCTION(BlueprintPure, Category = "NPC")
	ABikeActor* GetBike() const { return Bike; }

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	/** Places the actor on the circuit at the current distance and lane. */
	void UpdateTransform();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	/** Rider mesh; attached to the bike's Saddle socket at BeginPlay. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> RiderMesh;

	UPROPERTY(Transient)
	TObjectPtr<ABikeActor> Bike;

private:
	OpenRideCore::FNpcFollowerState State;
};
