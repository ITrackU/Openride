#include "Player/CyclistCharacter.h"

#include "Camera/CameraComponent.h"
#include "Circuit/CircuitActor.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Kismet/GameplayStatics.h"

ACyclistCharacter::ACyclistCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Root);
	SpringArm->TargetArmLength = 350.f;
	SpringArm->SocketOffset = FVector(0.f, 0.f, 120.f);
	// Mouse look drives the controller rotation; the arm follows it.
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void ACyclistCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!Circuit)
	{
		Circuit = Cast<ACircuitActor>(UGameplayStatics::GetActorOfClass(this, ACircuitActor::StaticClass()));
	}
	ApplyCameraMode();
}

void ACyclistCharacter::CreateDefaultInput()
{
	if (MappingContext)
	{
		return;
	}

	MappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Cyclist"));

	MoveAction = NewObject<UInputAction>(this, TEXT("IA_Move"));
	MoveAction->ValueType = EInputActionValueType::Axis1D;
	LookAction = NewObject<UInputAction>(this, TEXT("IA_Look"));
	LookAction->ValueType = EInputActionValueType::Axis2D;
	ZoomAction = NewObject<UInputAction>(this, TEXT("IA_Zoom"));
	ZoomAction->ValueType = EInputActionValueType::Axis1D;
	ToggleCameraAction = NewObject<UInputAction>(this, TEXT("IA_ToggleCamera"));
	ToggleCameraAction->ValueType = EInputActionValueType::Boolean;

	// W/Up = +1, S/Down = -1 via a Negate modifier.
	auto MapNegated = [this](const FKey& Key)
	{
		FEnhancedActionKeyMapping& Mapping = MappingContext->MapKey(MoveAction, Key);
		Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(MappingContext));
	};
	MappingContext->MapKey(MoveAction, EKeys::W);
	MappingContext->MapKey(MoveAction, EKeys::Up);
	MapNegated(EKeys::S);
	MapNegated(EKeys::Down);

	// Mouse X is yaw as-is; mouse Y needs negating so moving the mouse up looks up.
	MappingContext->MapKey(LookAction, EKeys::MouseX);
	FEnhancedActionKeyMapping& MouseY = MappingContext->MapKey(LookAction, EKeys::MouseY);
	MouseY.Modifiers.Add(NewObject<UInputModifierSwizzleAxis>(MappingContext)); // Y -> second component
	// Mouse Y is inverted at use time in OnLook (screen-space convention).

	MappingContext->MapKey(ZoomAction, EKeys::MouseScrollUp);
	FEnhancedActionKeyMapping& ZoomOut = MappingContext->MapKey(ZoomAction, EKeys::MouseScrollDown);
	ZoomOut.Modifiers.Add(NewObject<UInputModifierNegate>(MappingContext));

	MappingContext->MapKey(ToggleCameraAction, EKeys::C);
}

void ACyclistCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	CreateDefaultInput();
	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Subsystem->ClearAllMappings();
				Subsystem->AddMappingContext(MappingContext, 0);
			}
		}
	}
}

void ACyclistCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	CreateDefaultInput();
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACyclistCharacter::OnMove);
		EIC->BindAction(MoveAction, ETriggerEvent::Completed, this, &ACyclistCharacter::OnMove);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACyclistCharacter::OnLook);
		EIC->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ACyclistCharacter::OnZoom);
		EIC->BindAction(ToggleCameraAction, ETriggerEvent::Started, this, &ACyclistCharacter::OnToggleCamera);
	}
}

void ACyclistCharacter::OnMove(const FInputActionValue& Value)
{
	// On Completed the value is zero, which resets the axis.
	MoveAxis = Value.Get<float>();
}

void ACyclistCharacter::OnLook(const FInputActionValue& Value)
{
	const FVector2D Look = Value.Get<FVector2D>();
	AddControllerYawInput(Look.X);
	AddControllerPitchInput(-Look.Y);
}

void ACyclistCharacter::OnZoom(const FInputActionValue& Value)
{
	if (bFirstPerson)
	{
		return;
	}
	const float NewLength = SpringArm->TargetArmLength - Value.Get<float>() * ZoomStep;
	SpringArm->TargetArmLength = FMath::Clamp(NewLength, MinArmLength, MaxArmLength);
}

void ACyclistCharacter::OnToggleCamera(const FInputActionValue& Value)
{
	ToggleCameraMode();
}

void ACyclistCharacter::ToggleCameraMode()
{
	bFirstPerson = !bFirstPerson;
	ApplyCameraMode();
}

void ACyclistCharacter::ApplyCameraMode()
{
	// First person = zero-length arm at eye height; chase keeps the user-chosen zoom.
	if (bFirstPerson)
	{
		SpringArm->TargetArmLength = 0.f;
		SpringArm->SocketOffset = FVector(0.f, 0.f, 170.f);
		SpringArm->bDoCollisionTest = false;
	}
	else
	{
		SpringArm->TargetArmLength = FMath::Clamp(350.f, MinArmLength, MaxArmLength);
		SpringArm->SocketOffset = FVector(0.f, 0.f, 120.f);
		SpringArm->bDoCollisionTest = true;
	}
}

void ACyclistCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!Circuit)
	{
		return;
	}

	OpenRideCore::FCyclistMovementParams Params;
	Params.MaxSpeed = MaxSpeedMps;
	Params.Acceleration = AccelerationMps2;
	Params.BrakeDecel = BrakeDecelMps2;
	Params.CoastDecel = CoastDecelMps2;

	// Only the speed/advance is taken from the model; wrapping is done below on the
	// circuit length, so the model's own Distance is reset every frame.
	OpenRideCore::FCyclistMovementState State;
	State.Speed = SpeedMps;
	OpenRideCore::StepMovement(State, Params, FMath::Max(MoveAxis, 0.f), FMath::Max(-MoveAxis, 0.f), DeltaSeconds);
	SpeedMps = static_cast<float>(State.Speed);
	DistanceMeters += static_cast<float>(State.Distance);

	// Keep the distance in [0, L) so the value stays readable on a closed loop.
	const float Length = Circuit->GetLoopLengthMeters();
	if (Length > KINDA_SMALL_NUMBER)
	{
		DistanceMeters = FMath::Fmod(DistanceMeters, Length);
		if (DistanceMeters < 0.f)
		{
			DistanceMeters += Length;
		}
	}

	// Only the position follows the spline; orientation comes from mouse look via the controller.
	SetActorLocation(Circuit->GetTransformAtDistance(DistanceMeters).GetLocation());
}
