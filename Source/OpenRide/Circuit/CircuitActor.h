#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CircuitActor.generated.h"

class USplineComponent;

/**
 * Closed stadium-shaped circuit. Owns a spline built from the engine-independent
 * FStadiumCircuit math (core/circuit) so riders can follow the track by distance.
 */
UCLASS()
class OPENRIDE_API ACircuitActor : public AActor
{
	GENERATED_BODY()

public:
	ACircuitActor();

	/** Total loop length in metres. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Circuit", meta = (ClampMin = "100.0"))
	float LengthMeters = 400.f;

	/** Radius of the two semicircles in metres; straights are derived from it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Circuit", meta = (ClampMin = "5.0"))
	float RadiusMeters = 30.f;

	/** Road width in metres (used by the road mesh in a later task). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Circuit", meta = (ClampMin = "1.0"))
	float WidthMeters = 6.f;

	/** Number of spline points used to approximate the loop. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Circuit", meta = (ClampMin = "16", ClampMax = "512"))
	int32 NumSplinePoints = 64;

	/** World transform at a distance along the track; wraps around (closed loop). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Circuit")
	FTransform GetTransformAtDistance(float Meters) const;

	/** Spline length in metres. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Circuit")
	float GetLoopLengthMeters() const;

	UFUNCTION(BlueprintCallable, Category = "Circuit")
	void RebuildSpline();

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Circuit")
	TObjectPtr<USplineComponent> Spline;
};
