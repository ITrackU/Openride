#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "../../../core/anim/CyclistAnimModel.h"
#include "CyclistAnimInstance.generated.h"

/**
 * Feeds the rider Animation Blueprint (see docs/ANIMATION_BLUEPRINT_SPEC.md).
 * Motion is observed from the owning actor (the bike, the pawn or an NPC), so the
 * same class serves the player and every NPC without extra wiring.
 * The maths lives in core/anim (engine-independent, unit-tested); this class only
 * measures speed (from position change) and yaw rate and publishes the results as Blueprint-readable variables.
 */
UCLASS()
class OPENRIDE_API UCyclistAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	/** Forward speed in m/s. */
	UPROPERTY(BlueprintReadOnly, Category = "Cyclist")
	float Speed = 0.f;

	/** Crank cadence in rpm; 0 when coasting or stopped. Drives A_Cyc_Pedal play rate. */
	UPROPERTY(BlueprintReadOnly, Category = "Cyclist")
	float Cadence = 0.f;

	/** Body lean in [-1, 1]; negative = left, positive = right. */
	UPROPERTY(BlueprintReadOnly, Category = "Cyclist")
	float Lean = 0.f;

	/** True while out of the saddle. */
	UPROPERTY(BlueprintReadOnly, Category = "Cyclist")
	bool bSprinting = false;

	/** True while the legs are driving the cranks (false = idle or coasting). */
	UPROPERTY(BlueprintReadOnly, Category = "Cyclist")
	bool bPedaling = false;

	/** Play rate for crank-synchronised loops: 1 cycle = 1 crank turn, so rate = rpm / 60 (cycles/s) times the clip's length in s. */
	UFUNCTION(BlueprintPure, Category = "Cyclist")
	float GetPedalPlayRate(float AnimLengthSeconds) const { return Cadence / 60.f * AnimLengthSeconds; }

	/** Metres of travel per crank turn (gearing). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cyclist|Model", meta = (ClampMin = "1.0"))
	float GearDevelopmentMeters = 6.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cyclist|Model", meta = (ClampMin = "30.0"))
	float MaxCadenceRpm = 120.f;

	/** Cadence above which the rider stands up (leaves it 10 rpm lower). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cyclist|Model", meta = (ClampMin = "30.0"))
	float SprintCadenceRpm = 100.f;

	/** Banking angle in degrees that maps to Lean = +-1. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cyclist|Model", meta = (ClampMin = "1.0", ClampMax = "80.0"))
	float MaxLeanDegrees = 35.f;

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	OpenRideCore::FCyclistAnimState Model;
	float PreviousYawDegrees = 0.f;
	bool bHasPreviousYaw = false;
	FVector PreviousLocation = FVector::ZeroVector;
	bool bHasPreviousLocation = false;
};
