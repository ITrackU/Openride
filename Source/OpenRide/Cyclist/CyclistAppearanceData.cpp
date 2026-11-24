#include "CyclistAppearanceData.h"

#include "BikeActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "../../../core/appearance/AppearanceModel.h"

const FName UCyclistAppearanceData::JerseyParam(TEXT("JerseyColor"));
const FName UCyclistAppearanceData::ShortsParam(TEXT("ShortsColor"));
const FName UCyclistAppearanceData::HelmetParam(TEXT("HelmetColor"));
const FName UCyclistAppearanceData::FrameParam(TEXT("FrameColor"));

namespace
{
	/** Sets every colour on all slots; materials lacking a parameter simply ignore it. */
	void ApplyColors(USkeletalMeshComponent* Mesh, const TArray<TPair<FName, FLinearColor>>& Colors)
	{
		if (!Mesh)
		{
			return;
		}
		for (int32 Slot = 0; Slot < Mesh->GetNumMaterials(); ++Slot)
		{
			// Reuses an existing MID, so calling this again (e.g. re-skinning) does not leak instances.
			UMaterialInstanceDynamic* Mid = Mesh->CreateAndSetMaterialInstanceDynamic(Slot);
			if (!Mid)
			{
				continue;
			}
			for (const TPair<FName, FLinearColor>& Entry : Colors)
			{
				Mid->SetVectorParameterValue(Entry.Key, Entry.Value);
			}
		}
	}
}

void UCyclistAppearanceData::ApplyToRider(USkeletalMeshComponent* RiderMesh) const
{
	ApplyColors(RiderMesh, {
		{JerseyParam, JerseyColor},
		{ShortsParam, ShortsColor},
		{HelmetParam, HelmetColor}});
}

void UCyclistAppearanceData::ApplyToBike(ABikeActor* Bike) const
{
	if (Bike)
	{
		ApplyColors(Bike->BikeMesh, {{FrameParam, FrameColor}});
	}
}

UCyclistAppearanceData* UCyclistAppearanceData::MakeRandom(UObject* Outer, int32 Seed)
{
	UCyclistAppearanceData* Data = NewObject<UCyclistAppearanceData>(Outer ? Outer : GetTransientPackage());
	const OpenRideCore::FAppearance A = OpenRideCore::RandomAppearance(static_cast<uint32>(Seed));
	Data->JerseyColor = FLinearColor(A.Jersey.R, A.Jersey.G, A.Jersey.B);
	Data->ShortsColor = FLinearColor(A.Shorts.R, A.Shorts.G, A.Shorts.B);
	Data->HelmetColor = FLinearColor(A.Helmet.R, A.Helmet.G, A.Helmet.B);
	Data->FrameColor = FLinearColor(A.Frame.R, A.Frame.G, A.Frame.B);
	return Data;
}
