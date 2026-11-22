#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "CyclistCharacter.generated.h"

class ACircuitActor;
class UCameraComponent;
class USpringArmComponent;
class USceneComponent;
class UInputAction;
class UInputMappingContext;

/**
 * Viewer-mode player pawn. Moves along the circuit by distance (not by physics),
 * with mouse look, scroll zoom and a camera-mode toggle.
 * The real speed model (accelerate/brake) comes in the next task; here a simple
 * constant-speed placeholder keeps the pawn testable.
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

	/** Placeholder speed (m/s) while a move key is held; replaced by the movement model task. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float PlaceholderSpeedMps = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "50.0"))
	float MinArmLength = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (ClampMin = "50.0"))
	float MaxArmLength = 800.f;

	/** Arm length change per scroll notch, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ZoomStep = 40.f;

	/** Toggles between chase (third person) and first person camera. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ToggleCameraMode();

	UFUNCTION(BlueprintPure, Category = "Camera")
	bool IsFirstPerson() const { return bFirstPerson; }

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;
	virtual void PawnClientRestart() override;

	void OnMove(const FInputActionValue& Value);
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
	UPROPERTY() TObjectPtr<UInputAction> LookAction;
	UPROPERTY() TObjectPtr<UInputAction> ZoomAction;
	UPROPERTY() TObjectPtr<UInputAction> ToggleCameraAction;

private:
	bool bFirstPerson = false;
	/** Forward/back input in [-1, 1] captured this frame. */
	float MoveAxis = 0.f;
};
