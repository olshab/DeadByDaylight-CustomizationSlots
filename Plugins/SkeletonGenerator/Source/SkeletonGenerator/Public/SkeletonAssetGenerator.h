#pragma once

#include "CoreMinimal.h"

class UGenerationSettings;
class FJsonObject;
class USkeleton;

class FSkeletonAssetGenerator
{
public:
	FSkeletonAssetGenerator(UGenerationSettings* Settings) : 
		Settings(Settings),
		Skeleton(NULL)
	{ }

	bool Generate();

private:
	USkeleton* CreateSkeletonObject(const FString& PackagePath, const FString& AssetName);

	void GenerateBoneTree();
	void GenerateReferenceSkeleton();
	void GenerateGuid();
	void GenerateSlotGroups();
	void GenerateSockets();

private:
	UGenerationSettings* Settings;
	TSharedPtr<FJsonObject> AssetJsonObject;
	USkeleton* Skeleton;
};
