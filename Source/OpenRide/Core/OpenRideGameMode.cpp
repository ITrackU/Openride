#include "Core/OpenRideGameMode.h"

#include "Player/CyclistCharacter.h"
#include "UI/OpenRideHUD.h"

AOpenRideGameMode::AOpenRideGameMode()
{
	DefaultPawnClass = ACyclistCharacter::StaticClass();
	HUDClass = AOpenRideHUD::StaticClass();
}
