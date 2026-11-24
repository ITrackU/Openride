#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OpenRideGameMode.generated.h"

/** Viewer-mode game mode: spawns the cyclist pawn and shows the HUD. */
UCLASS()
class OPENRIDE_API AOpenRideGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AOpenRideGameMode();
};
