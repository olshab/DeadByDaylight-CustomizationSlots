#pragma once 

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "TestSettings.generated.h"

UCLASS(Config=Editor, DefaultConfig, meta=(DisplayName="TestWidget"))
class UTestSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	FORCEINLINE static UTestSettings* Get()
	{
		return GetMutableDefault<UTestSettings>();
	}

public:
	UPROPERTY(EditAnywhere, Config)
	FString DumpedAssetsPath;

	UPROPERTY(EditAnywhere, Config, meta=(AllowedClasses="SkeletalMesh"))
	FSoftObjectPath Mesh;
};
