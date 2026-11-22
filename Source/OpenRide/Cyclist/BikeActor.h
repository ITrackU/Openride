#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../../core/bike/BikeModel.h"
#include "BikeActor.generated.h"

class USceneComponent;
class USkeletalMesh;
class USkeletalMeshComponent;

/**
 * The shared bike (see docs/CYCLIST_PIPELINE.md): used by the player and every NPC.
 * Motion is measured from the actor's own movement (like UCyclistAnimInstance), so whoever
 * moves the bike (player pawn, NPC, or a parent actor it is attached to) needs no extra wiring.
 *  - Frame roll (lean) is applied to a pivot component here.
 *  - Wheel spin and fork steering are bone rotations, applied by UBikeAnimInstance (ABP_Bike).
 * The maths lives in core/bike (engine-independent, unit-tested).
 */
UCLASS()
class OPENRIDE_API ABikeActor : public AActor
{
	GENERATED_BODY()

public:
	ABikeActor();

	/** Rolls with Lean so rider and bike tilt together; the bike mesh and the rider hang below it. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bike")
	TObjectPtr<USceneComponent> LeanPivot;

	/** SK_Bike. Assign the mesh and ABP_Bike (UBikeAnimInstance) in a Blueprint subclass. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bike")
	TObjectPtr<USkeletalMeshComponent> BikeMesh;

	/** Frame roll in degrees at full lean (the model clamps at this). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bike|Model", meta = (ClampMin = "0.0", ClampMax = "60.0"))
	float MaxLeanDegrees = 30.f;

	/** Fork yaw limit in degrees; purely visual. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bike|Model", meta = (ClampMin = "0.0", ClampMax = "60.0"))
	float MaxSteerDegrees = 28.f;

	/** Rolling radius in cm; sets how fast the wheels spin for a given speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bike|Model", meta = (ClampMin = "10.0"))
	float WheelRadiusCm = 34.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bike|Model", meta = (ClampMin = "50.0"))
	float WheelbaseCm = 100.f;

	/** Accumulated wheel rotation in degrees [0, 360); read by the anim instance. */
	UFUNCTION(BlueprintPure, Category = "Bike")
	float GetWheelAngleDegrees() const { return FMath::RadiansToDegrees(State.WheelAngle); }

	/** Fork yaw in degrees, positive = right. */
	UFUNCTION(BlueprintPure, Category = "Bike")
	float GetSteerDegrees() const { return FMath::RadiansToDegrees(State.Steer); }

	/** Frame roll in degrees, positive = right. */
	UFUNCTION(BlueprintPure, Category = "Bike")
	float GetLeanDegrees() const { return FMath::RadiansToDegrees(State.Lean); }

	/** Forward speed in m/s as measured from movement. */
	UFUNCTION(BlueprintPure, Category = "Bike")
	float GetSpeedMps() const { return MeasuredSpeedMps; }

	/** Attaches the rider mesh to the Saddle socket (pelvis aligned, see pipeline doc). */
	UFUNCTION(BlueprintCallable, Category = "Bike")
	void AttachRider(USkeletalMeshComponent* RiderMesh);

	/** World location of a bike socket (HandLeft, HandRight, FootLeft, FootRight, Saddle, ...) for rider IK. */
	UFUNCTION(BlueprintPure, Category = "Bike")
	FVector GetSocketWorldLocation(FName SocketName) const;

	static const FName SaddleSocket;
	static const FName HandLeftSocket;
	static const FName HandRightSocket;
	static const FName FootLeftSocket;
	static const FName FootRightSocket;

	virtual void Tick(float DeltaSeconds) override;

private:
	OpenRideCore::FBikeState State;
	float MeasuredSpeedMps = 0.f;
	FVector PreviousLocation = FVector::ZeroVector;
	float PreviousYawDegrees = 0.f;
	bool bHasPrevious = false;
};
