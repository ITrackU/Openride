#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CircuitActor.generated.h"

class USplineComponent;
class UProceduralMeshComponent;
class UMaterialInterface;

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

	/** Road width in metres. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Circuit", meta = (ClampMin = "1.0"))
	float WidthMeters = 6.f;

	/** Number of spline points used to approximate the loop. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Circuit", meta = (ClampMin = "16", ClampMax = "512"))
	int32 NumSplinePoints = 64;

	/** Dashed centre line length / gap in metres. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Road", meta = (ClampMin = "0.5"))
	float DashLengthMeters = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Road", meta = (ClampMin = "0.0"))
	float DashGapMeters = 3.f;

	/** Width of the lane marking lines in metres. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Road", meta = (ClampMin = "0.05"))
	float MarkingWidthMeters = 0.15f;

	/** Distance of the solid edge lines from the road border, in metres. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Road", meta = (ClampMin = "0.0"))
	float EdgeLineInsetMeters = 0.3f;

	/** Mesh sampling step in metres; smaller = smoother bends, more triangles. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Road", meta = (ClampMin = "0.1"))
	float MeshStepMeters = 0.5f;

	/** Asphalt material; falls back to the engine grid material until a real one is assigned. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
	TObjectPtr<UMaterialInterface> RoadMaterial;

	/** Lane marking material; falls back to the engine grid material. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Road")
	TObjectPtr<UMaterialInterface> MarkingMaterial;

	/** Regenerates road and lane marking meshes from the circuit math. */
	UFUNCTION(BlueprintCallable, Category = "Circuit")
	void RebuildRoadMesh();

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road")
	TObjectPtr<UProceduralMeshComponent> RoadMesh;

	/** Separate component so markings can use their own (e.g. white, unlit) material. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Road")
	TObjectPtr<UProceduralMeshComponent> MarkingMesh;
};
