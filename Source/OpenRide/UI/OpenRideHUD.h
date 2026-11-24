#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "../../../core/menu/MenuModel.h"
#include "../../../core/settings/SettingsModel.h"
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
	enum class EScreen : uint8 { Pause, Settings };
	// Settings rows: 0 resolution, 1 quality, 2.. one per rebindable action, last = Back.
	static constexpr int32 SettingsResolutionRow = 0;
	static constexpr int32 SettingsQualityRow = 1;
	static constexpr int32 SettingsFirstKeyRow = 2;

	const std::vector<OpenRideCore::FMenuItem>& ActiveItems() const;
	void BuildSettingsItems();
	void OpenSettingsScreen();
	void ActivateSettingsItem(int32 Index);
	void FinishRebind(const FKey& Key);
	void PollRebindKey();

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
	EScreen Screen = EScreen::Pause;
	/** Action waiting for its new key, or empty when not capturing. */
	std::string RebindAction;
	uint64 RebindStartFrame = 0;
	std::vector<OpenRideCore::FResolution> Resolutions;
	int32 ResolutionIndex = 0;
	OpenRideCore::FKeyBindings KeyBindings = OpenRideCore::FKeyBindings::Defaults();
	std::vector<OpenRideCore::FMenuItem> SettingsItems;
	int32 SelectedItem = 0;
	std::vector<OpenRideCore::FMenuItem> PauseItems;
};
