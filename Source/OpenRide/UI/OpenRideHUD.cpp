#include "UI/OpenRideHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Components/InputComponent.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/CyclistCharacter.h"
#include "UI/OpenRideSettingsLibrary.h"

FString AOpenRideHUD::FormatTime(float Seconds)
{
	if (Seconds < 0.f)
	{
		return TEXT("--:--.---");
	}
	const int32 TotalMs = FMath::RoundToInt(Seconds * 1000.f);
	return FString::Printf(TEXT("%d:%02d.%03d"), TotalMs / 60000, (TotalMs / 1000) % 60, TotalMs % 1000);
}

void AOpenRideHUD::BeginPlay()
{
	Super::BeginPlay();
	BuildPauseItems();

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}
	// The HUD owns its own input component so the menu works while the game is paused
	// (bExecuteWhenPaused) and needs no input assets.
	EnableInput(PC);
	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AOpenRideHUD::TogglePauseMenu).bExecuteWhenPaused = true;
		InputComponent->BindKey(EKeys::Up, IE_Pressed, this, &AOpenRideHUD::OnMenuUp).bExecuteWhenPaused = true;
		InputComponent->BindKey(EKeys::Down, IE_Pressed, this, &AOpenRideHUD::OnMenuDown).bExecuteWhenPaused = true;
		InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &AOpenRideHUD::OnMenuConfirm).bExecuteWhenPaused = true;
		InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AOpenRideHUD::OnMouseClick).bExecuteWhenPaused = true;
		// Keep the key events flowing to the pawn too (Up/Down also drive the bike when the menu is closed).
		for (FInputKeyBinding& Binding : InputComponent->KeyBindings)
		{
			Binding.bConsumeInput = false;
		}
	}
}

void AOpenRideHUD::BuildPauseItems()
{
	PauseItems = { {"Resume", true}, {"Settings", true}, {"Quit", true} };
}

const std::vector<OpenRideCore::FMenuItem>& AOpenRideHUD::ActiveItems() const
{
	return Screen == EScreen::Settings ? SettingsItems : PauseItems;
}

void AOpenRideHUD::BuildSettingsItems()
{
	SettingsItems.clear();
	const OpenRideCore::FResolution& R = Resolutions[ResolutionIndex];
	SettingsItems.push_back({"Resolution: " + std::to_string(R.Width) + "x" + std::to_string(R.Height), true});

	const int32 Q = UOpenRideSettingsLibrary::GetQualityLevel();
	SettingsItems.push_back({"Quality: " + std::string(Q < 0 ? "Custom" : TCHAR_TO_UTF8(*UOpenRideSettingsLibrary::GetQualityName(Q))), true});

	// Pause (Escape) is not rebindable: Escape is also the menu's cancel key.
	for (const OpenRideCore::FKeyBinding& B : KeyBindings.All())
	{
		if (B.Action == "Pause")
		{
			continue;
		}
		const bool bCapturing = (RebindAction == B.Action);
		SettingsItems.push_back({B.Action + ": " + (bCapturing ? "press a key..." : B.Key), true});
	}
	SettingsItems.push_back({"Back", true});
}

void AOpenRideHUD::OpenSettingsScreen()
{
	Resolutions = OpenRideCore::DefaultResolutions();
	const FIntPoint Cur = GEngine && GEngine->GameUserSettings ? GEngine->GameUserSettings->GetScreenResolution() : FIntPoint(1280, 720);
	ResolutionIndex = FMath::Max(0, OpenRideCore::NearestResolutionIndex(Resolutions, {Cur.X, Cur.Y}));
	Screen = EScreen::Settings;
	SelectedItem = 0;
	RebindAction.clear();
	BuildSettingsItems();
}

void AOpenRideHUD::ActivateSettingsItem(int32 Index)
{
	const int32 Last = static_cast<int32>(SettingsItems.size()) - 1;
	if (Index == Last)
	{
		UOpenRideSettingsLibrary::SaveSettings();
		Screen = EScreen::Pause;
		SelectedItem = Settings;
		return;
	}
	if (Index == SettingsResolutionRow)
	{
		ResolutionIndex = (ResolutionIndex + 1) % static_cast<int32>(Resolutions.size());
		UOpenRideSettingsLibrary::ApplyResolution(FIntPoint(Resolutions[ResolutionIndex].Width, Resolutions[ResolutionIndex].Height));
	}
	else if (Index == SettingsQualityRow)
	{
		// Custom (-1) wraps to Low via (-1 + 1) % 4.
		UOpenRideSettingsLibrary::ApplyQualityLevel((UOpenRideSettingsLibrary::GetQualityLevel() + 1) % 4);
	}
	else
	{
		// Rows from SettingsFirstKeyRow map to the non-Pause actions in table order.
		int32 Row = SettingsFirstKeyRow;
		for (const OpenRideCore::FKeyBinding& B : KeyBindings.All())
		{
			if (B.Action == "Pause") { continue; }
			if (Row++ == Index) { RebindAction = B.Action; RebindStartFrame = GFrameCounter; break; }
		}
	}
	BuildSettingsItems();
}

void AOpenRideHUD::FinishRebind(const FKey& Key)
{
	if (Key != EKeys::Escape) // Escape cancels the capture.
	{
		KeyBindings.Rebind(RebindAction, TCHAR_TO_UTF8(*Key.GetFName().ToString()));
		if (ACyclistCharacter* Cyclist = Cast<ACyclistCharacter>(GetOwningPawn()))
		{
			Cyclist->ApplyKeyBindings(KeyBindings);
		}
	}
	RebindAction.clear();
	BuildSettingsItems();
}

void AOpenRideHUD::PollRebindKey()
{
	// Polled instead of bound: a single AnyKey binding would not tell us which key was pressed.
	APlayerController* PC = GetOwningPlayerController();
	// Skip the activation frame, or the Enter press that started the capture would be taken as the new key.
	if (!PC || GFrameCounter <= RebindStartFrame)
	{
		return;
	}
	TArray<FKey> AllKeys;
	EKeys::GetAllKeys(AllKeys);
	for (const FKey& Key : AllKeys)
	{
		if (Key.IsMouseButton() || Key.IsGamepadKey() || Key.IsAxis1D() || Key.IsAxis2D() || Key.IsAxis3D() || !Key.IsValid())
		{
			continue;
		}
		if (PC->WasInputKeyJustPressed(Key))
		{
			FinishRebind(Key);
			return;
		}
	}
}

OpenRideCore::FMenuLayout AOpenRideHUD::MakeLayout() const
{
	OpenRideCore::FMenuLayout Layout;
	if (Canvas)
	{
		Layout.ScreenW = Canvas->SizeX;
		Layout.ScreenH = Canvas->SizeY;
		Layout.ButtonW = Canvas->SizeX * 0.25f;
		Layout.ButtonH = Canvas->SizeY * 0.07f;
		Layout.Gap = Canvas->SizeY * 0.02f;
	}
	return Layout;
}

void AOpenRideHUD::SetPauseMenuOpen(bool bOpen)
{
	bPauseMenuOpen = bOpen;
	Screen = EScreen::Pause;
	RebindAction.clear();
	SelectedItem = 0;
	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->bShowMouseCursor = bOpen;
		if (bOpen)
		{
			PC->SetInputMode(FInputModeGameAndUI());
		}
		else
		{
			PC->SetInputMode(FInputModeGameOnly());
		}
	}
	UGameplayStatics::SetGamePaused(this, bOpen);
}

void AOpenRideHUD::TogglePauseMenu()
{
	if (bPauseMenuOpen && Screen == EScreen::Settings)
	{
		// Escape inside settings: cancel a pending rebind (handled by the poll), else go back.
		if (RebindAction.empty())
		{
			UOpenRideSettingsLibrary::SaveSettings();
			Screen = EScreen::Pause;
			SelectedItem = Settings;
		}
		return;
	}
	SetPauseMenuOpen(!bPauseMenuOpen);
}

void AOpenRideHUD::OnMenuUp()
{
	if (bPauseMenuOpen && RebindAction.empty())
	{
		SelectedItem = OpenRideCore::MoveSelection(ActiveItems(), SelectedItem, -1);
	}
}

void AOpenRideHUD::OnMenuDown()
{
	if (bPauseMenuOpen && RebindAction.empty())
	{
		SelectedItem = OpenRideCore::MoveSelection(ActiveItems(), SelectedItem, +1);
	}
}

void AOpenRideHUD::OnMenuConfirm()
{
	if (bPauseMenuOpen && RebindAction.empty())
	{
		ActivateItem(SelectedItem);
	}
}

void AOpenRideHUD::OnMouseClick()
{
	APlayerController* PC = GetOwningPlayerController();
	float X = 0.f, Y = 0.f;
	if (!bPauseMenuOpen || !RebindAction.empty() || !PC || !PC->GetMousePosition(X, Y))
	{
		return;
	}
	const int32 Hit = OpenRideCore::HitTest(MakeLayout(), ActiveItems(), X, Y);
	if (Hit >= 0)
	{
		ActivateItem(Hit);
	}
}

void AOpenRideHUD::ActivateItem(int32 Index)
{
	if (Screen == EScreen::Settings)
	{
		ActivateSettingsItem(Index);
		return;
	}
	switch (Index)
	{
	case Resume:
		SetPauseMenuOpen(false);
		break;
	case Quit:
		UKismetSystemLibrary::QuitGame(this, GetOwningPlayerController(), EQuitPreference::Quit, false);
		break;
	case Settings:
		OpenSettingsScreen();
		break;
	default:
		break;
	}
}

void AOpenRideHUD::DrawPauseMenu()
{
	if (!Canvas || !GEngine)
	{
		return;
	}
	// Dim the world behind the menu.
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.6f), 0.f, 0.f, Canvas->SizeX, Canvas->SizeY);

	UFont* Font = GEngine->GetMediumFont();
	const OpenRideCore::FMenuLayout Layout = MakeLayout();
	const std::vector<OpenRideCore::FMenuItem>& Items = ActiveItems();
	const int32 Count = static_cast<int32>(Items.size());

	const FString Title = Screen == EScreen::Settings ? TEXT("Settings") : TEXT("Paused");
	float TW = 0.f, TH = 0.f;
	GetTextSize(Title, TW, TH, Font, TextScale * 1.5f);
	DrawText(Title, FLinearColor::White, (Canvas->SizeX - TW) * 0.5f,
		OpenRideCore::ButtonRect(Layout, Count, 0).Y - TH - Layout.Gap * 2.f, Font, TextScale * 1.5f);

	for (int32 i = 0; i < Count; ++i)
	{
		const OpenRideCore::FMenuItem& Item = Items[i];
		const OpenRideCore::FRect R = OpenRideCore::ButtonRect(Layout, Count, i);
		const bool bSelected = (i == SelectedItem) && Item.bEnabled;
		DrawRect(bSelected ? FLinearColor(0.2f, 0.5f, 0.9f, 0.9f) : FLinearColor(0.1f, 0.1f, 0.1f, 0.85f),
			R.X, R.Y, R.W, R.H);

		const FString Label(UTF8_TO_TCHAR(Item.Label.c_str()));
		float W = 0.f, H = 0.f;
		GetTextSize(Label, W, H, Font, TextScale);
		DrawText(Label, Item.bEnabled ? FLinearColor::White : FLinearColor(0.5f, 0.5f, 0.5f, 1.f),
			R.X + (R.W - W) * 0.5f, R.Y + (R.H - H) * 0.5f, Font, TextScale);
	}
}

void AOpenRideHUD::DrawHUD()
{
	Super::DrawHUD();

	if (bPauseMenuOpen)
	{
		if (!RebindAction.empty())
		{
			PollRebindKey();
		}
		DrawPauseMenu();
		return;
	}

	const ACyclistCharacter* Cyclist = Cast<ACyclistCharacter>(GetOwningPawn());
	if (!Cyclist || !Canvas || !GEngine)
	{
		return;
	}

	const float SpeedKmh = Cyclist->SpeedMps * 3.6f;
	const FString Lines[] = {
		FString::Printf(TEXT("%.1f km/h"), SpeedKmh),
		FString::Printf(TEXT("Distance: %.2f km"), Cyclist->GetTotalDistanceMeters() / 1000.f),
		FString::Printf(TEXT("Lap: %d"), Cyclist->GetLapCount() + 1),
		FString::Printf(TEXT("Lap time: %s"), *FormatTime(Cyclist->GetLapTimeSeconds())),
		FString::Printf(TEXT("Last: %s   Best: %s"),
			*FormatTime(Cyclist->GetLastLapTimeSeconds()), *FormatTime(Cyclist->GetBestLapTimeSeconds())),
	};

	UFont* Font = GEngine->GetMediumFont();
	float Y = Canvas->SizeY * 0.03f;
	const float X = Canvas->SizeX * 0.02f;
	for (const FString& Line : Lines)
	{
		float W = 0.f, H = 0.f;
		GetTextSize(Line, W, H, Font, TextScale);
		DrawText(Line, FLinearColor::White, X, Y, Font, TextScale);
		Y += H + 4.f;
	}
}
