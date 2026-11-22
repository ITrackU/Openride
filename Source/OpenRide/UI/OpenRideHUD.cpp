#include "UI/OpenRideHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
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

void AOpenRideHUD::DrawHUD()
{
	Super::DrawHUD();

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
