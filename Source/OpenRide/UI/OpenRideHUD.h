#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "../../../core/menu/MenuModel.h"
#include "OpenRideHUD.generated.h"

/**
 * Minimal viewer-mode HUD: speed (km/h), distance, lap counter, lap time.
 * Uses AHUD canvas drawing (no UMG assets) so it works without any .uasset.
 */
UCLASS()
class OPENRIDE_API AOpenRideHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

	/** Opens/closes the pause menu and pauses/unpauses the game (bound to Escape). */
	void TogglePauseMenu();
	bool IsPauseMenuOpen() const { return bPauseMenuOpen; }

	/** Text scale; raise for high-DPI screens. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD", meta = (ClampMin = "0.5"))
	float TextScale = 1.5f;

	/** Formats seconds as M:SS.mmm; negative values give "--:--.---". Static so it is easy to test. */
	static FString FormatTime(float Seconds);

private:
	enum EPauseItem { Resume = 0, Settings = 1, Quit = 2 };

	void BuildPauseItems();
	void DrawPauseMenu();
	void ActivateItem(int32 Index);
	void OnMenuUp();
	void OnMenuDown();
	void OnMenuConfirm();
	void OnMouseClick();
	void SetPauseMenuOpen(bool bOpen);
	OpenRideCore::FMenuLayout MakeLayout() const;

	bool bPauseMenuOpen = false;
	int32 SelectedItem = 0;
	std::vector<OpenRideCore::FMenuItem> PauseItems;
};
