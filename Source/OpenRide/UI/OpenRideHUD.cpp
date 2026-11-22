#include "UI/OpenRideHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/CyclistCharacter.h"

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
	// Settings stays disabled until the settings screen task lands.
	PauseItems = { {"Resume", true}, {"Settings", false}, {"Quit", true} };
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
	SetPauseMenuOpen(!bPauseMenuOpen);
}

void AOpenRideHUD::OnMenuUp()
{
	if (bPauseMenuOpen)
	{
		SelectedItem = OpenRideCore::MoveSelection(PauseItems, SelectedItem, -1);
	}
}

void AOpenRideHUD::OnMenuDown()
{
	if (bPauseMenuOpen)
	{
		SelectedItem = OpenRideCore::MoveSelection(PauseItems, SelectedItem, +1);
	}
}

void AOpenRideHUD::OnMenuConfirm()
{
	if (bPauseMenuOpen)
	{
		ActivateItem(SelectedItem);
	}
}

void AOpenRideHUD::OnMouseClick()
{
	APlayerController* PC = GetOwningPlayerController();
	float X = 0.f, Y = 0.f;
	if (!bPauseMenuOpen || !PC || !PC->GetMousePosition(X, Y))
	{
		return;
	}
	const int32 Hit = OpenRideCore::HitTest(MakeLayout(), PauseItems, X, Y);
	if (Hit >= 0)
	{
		ActivateItem(Hit);
	}
}

void AOpenRideHUD::ActivateItem(int32 Index)
{
	switch (Index)
	{
	case Resume:
		SetPauseMenuOpen(false);
		break;
	case Quit:
		UKismetSystemLibrary::QuitGame(this, GetOwningPlayerController(), EQuitPreference::Quit, false);
		break;
	default:
		break; // Settings: not implemented yet (disabled in BuildPauseItems).
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
	const int32 Count = static_cast<int32>(PauseItems.size());

	const FString Title = TEXT("Paused");
	float TW = 0.f, TH = 0.f;
	GetTextSize(Title, TW, TH, Font, TextScale * 1.5f);
	DrawText(Title, FLinearColor::White, (Canvas->SizeX - TW) * 0.5f,
		OpenRideCore::ButtonRect(Layout, Count, 0).Y - TH - Layout.Gap * 2.f, Font, TextScale * 1.5f);

	for (int32 i = 0; i < Count; ++i)
	{
		const OpenRideCore::FMenuItem& Item = PauseItems[i];
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
