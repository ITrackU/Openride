#include "BikeAnimInstance.h"

#include "BikeActor.h"

void UBikeAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (const ABikeActor* Bike = Cast<ABikeActor>(GetOwningActor()))
	{
		WheelAngleDegrees = Bike->GetWheelAngleDegrees();
		SteerDegrees = Bike->GetSteerDegrees();
	}
}
