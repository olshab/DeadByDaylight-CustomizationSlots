#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GenerationSettings.generated.h"

UCLASS(Config=Editor, DefaultConfig, meta=(DisplayName="Skeleton Generation Settings"))
class UGenerationSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGenerationSettings();

	FORCEINLINE static UGenerationSettings* Get()
	{
		return GetMutableDefault<UGenerationSettings>();
	}

public:
	UPROPERTY(EditAnywhere, Config, meta=(DisplayName="Path To Dumped JSON File"))
	FString JsonFilePath;

	/**
	 * i.e.: /Game/Meshes/Physics
	 * Notice the use of "/Game" instead of "Content"
	 */
	UPROPERTY(EditAnywhere, Config, meta=(DisplayName="Skeleton Generation Game Folder"))
	FString GenerationFolder;

	UPROPERTY(EditAnywhere, Config, meta=(EditCondition="!bUseAssociatedMesh || !bUseDefaultAssetName"))
	FString AssetName;

	UPROPERTY(EditAnywhere, Config)
	bool bGenerateSlots;

	UPROPERTY(EditAnywhere, Config)
	bool bGenerateSockets;

	UPROPERTY(EditAnywhere, Config, Category = SkeletalMesh, meta = (DisplayName = "Associate With Mesh"))
	bool bUseAssociatedMesh;

	/**
	 * Will use default skeleton asset name for your skeletal mesh
	 * i.e.: MySkeletalMesh_Skeleton.uasset
	 */
	UPROPERTY(EditAnywhere, Config, Category=SkeletalMesh, meta=(EditCondition="bUseAssociatedMesh"))
	bool bUseDefaultAssetName;

	UPROPERTY(EditAnywhere, Config, Category = SkeletalMesh, meta=(AllowedClasses="SkeletalMesh", EditCondition="bUseAssociatedMesh"))
	FSoftObjectPath AssociatedMesh;
};
