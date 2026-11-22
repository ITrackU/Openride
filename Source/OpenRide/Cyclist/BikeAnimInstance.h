#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BikeAnimInstance.generated.h"

/**
 * Feeds ABP_Bike: rotates `wheel_front`/`wheel_rear` and the `fork` bone.
 * Values are read from the owning ABikeActor so the Blueprint needs only Transform (Modify) Bone
 * nodes (spec in docs/ANIMATION_BLUEPRINT_SPEC.md).
 */
UCLASS()
class OPENRIDE_API UBikeAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	/** Wheel spin about the axle in degrees; apply to both wheel bones (axle = bone local Y in the pipeline spec). */
	UPROPERTY(BlueprintReadOnly, Category = "Bike")
	float WheelAngleDegrees = 0.f;

	/** Fork yaw in degrees, positive = right; apply to the `fork` bone about its steering axis (local Z). */
	UPROPERTY(BlueprintReadOnly, Category = "Bike")
	float SteerDegrees = 0.f;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
