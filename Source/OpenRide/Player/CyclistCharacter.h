#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "../../../core/movement/CyclistMovement.h"
#include "CyclistCharacter.generated.h"

class ACircuitActor;
class UCameraComponent;
class USpringArmComponent;
class USceneComponent;
class UInputAction;
class UInputMappingContext;

/** Runtime-switchable camera modes. */
UENUM(BlueprintType)
enum class ECyclistCameraMode : uint8
{
	Chase,
	FirstPerson,
	/** Debug camera: detaches from the circuit and flies freely. */
	FreeFly
};

/**
 * Viewer-mode player pawn. Moves along the circuit by distance (not by physics),
 * with mouse look, scroll zoom and a camera-mode toggle.
 * Speed comes from the engine-independent OpenRideCore::StepMovement model
 * (W/Up pedals, S/Down brakes, releasing coasts).
 *
 * It is a APawn rather than ACharacter: we never need CharacterMovement because
 * the circuit spline dictates the position.
 */
UCLASS()
class OPENRIDE_API ACyclistCharacter : public APawn
{
	GENERATED_BODY()

public:
	ACyclistCharacter();

	/** Circuit to follow; auto-detected (first ACircuitActor in the level) when null. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Circuit")
	TObjectPtr<ACircuitActor> Circuit;

	/** Current distance along the circuit in metres. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Circuit")
	float DistanceMeters = 0.f;

	/** Top speed in m/s (14 = ~50 km/h). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.1"))
	float MaxSpeedMps = 14.f;

	/** Acceleration in m/s^2 at standstill; tapers off towards max speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float AccelerationMps2 = 2.f;

	/** Deceleration in m/s^2 while braking. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float BrakeDecelMps2 = 6.f;

	/** Deceleration in m/s^2 while coasting (rolling + air drag). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float CoastDecelMps2 = 0.4f;

	/** Current speed in m/s (read-only, for HUD/animation). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float SpeedMps = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "50.0"))
	float MinArmLength = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "50.0"))
	float MaxArmLength = 800.f;

	/** Arm length change per scroll notch, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ZoomStep = 40.f;

	/** Free-fly speed in cm/s (WASD = move, E/Q = up/down). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "1.0"))
	float FlySpeed = 1500.f;

	/** Cycles Chase -> FirstPerson -> FreeFly -> Chase. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ToggleCameraMode();

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraMode(ECyclistCameraMode NewMode);

	UFUNCTION(BlueprintPure, Category = "Camera")
	ECyclistCameraMode GetCameraMode() const { return CameraMode; }

	UFUNCTION(BlueprintPure, Category = "Camera")
	bool IsFirstPerson() const { return CameraMode == ECyclistCameraMode::FirstPerson; }

	UFUNCTION(BlueprintPure, Category = "Camera")
	bool IsFreeFly() const { return CameraMode == ECyclistCameraMode::FreeFly; }

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;
	virtual void PawnClientRestart() override;

	void OnMove(const FInputActionValue& Value);
	void OnFlyRight(const FInputActionValue& Value);
	void OnFlyUp(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnZoom(const FInputActionValue& Value);
	void OnToggleCamera(const FInputActionValue& Value);

	/** Applies the camera mode to the spring arm / camera attachment. */
	void ApplyCameraMode();

	/** Builds input actions and the mapping context in code so no .uasset is needed. */
	void CreateDefaultInput();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY() TObjectPtr<UInputMappingContext> MappingContext;
	UPROPERTY() TObjectPtr<UInputAction> MoveAction;
	UPROPERTY() TObjectPtr<UInputAction> FlyRightAction;
	UPROPERTY() TObjectPtr<UInputAction> FlyUpAction;
	UPROPERTY() TObjectPtr<UInputAction> LookAction;
	UPROPERTY() TObjectPtr<UInputAction> ZoomAction;
	UPROPERTY() TObjectPtr<UInputAction> ToggleCameraAction;

private:
	ECyclistCameraMode CameraMode = ECyclistCameraMode::Chase;
	/** Forward/back input in [-1, 1] captured this frame. */
	float MoveAxis = 0.f;
	/** Free-fly strafe and vertical inputs in [-1, 1]. */
	float FlyRightAxis = 0.f;
	float FlyUpAxis = 0.f;
};
