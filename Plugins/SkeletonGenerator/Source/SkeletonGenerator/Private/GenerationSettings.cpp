#include "GenerationSettings.h"

UGenerationSettings::UGenerationSettings()
	: Super()
{
	GenerationFolder = TEXT("/Game/Skeleton");
	AssetName = TEXT("UncookedSkeleton");
	bGenerateSlots = true;
	bGenerateSockets = true;
}
