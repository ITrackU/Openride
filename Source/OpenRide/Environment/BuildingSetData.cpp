#include "Environment/BuildingSetData.h"

void UBuildingSetData::FillDefaults()
{
	Variants.Reset();

	FBuildingVariant SmallHouse;
	SmallHouse.WidthRangeMeters = FVector2D(7.f, 10.f);
	SmallHouse.DepthRangeMeters = FVector2D(7.f, 9.f);
	SmallHouse.HeightRangeMeters = FVector2D(5.f, 7.f);
	SmallHouse.Weight = 3.f;
	Variants.Add(SmallHouse);

	FBuildingVariant House;
	House.WidthRangeMeters = FVector2D(10.f, 14.f);
	House.DepthRangeMeters = FVector2D(8.f, 11.f);
	House.HeightRangeMeters = FVector2D(7.f, 10.f);
	House.Weight = 2.f;
	Variants.Add(House);

	FBuildingVariant Block;
	Block.WidthRangeMeters = FVector2D(14.f, 22.f);
	Block.DepthRangeMeters = FVector2D(10.f, 14.f);
	Block.HeightRangeMeters = FVector2D(12.f, 24.f);
	Block.Weight = 1.f;
	Variants.Add(Block);
}
