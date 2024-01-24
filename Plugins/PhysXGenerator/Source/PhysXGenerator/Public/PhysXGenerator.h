// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;
class FJsonValue;
class FJsonObject;
class UPhysicsAsset;
class USkeletalBodySetup;
class UPhysicsConstraintTemplate;

class FPhysXGeneratorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();
	
private:

	void RegisterMenus();

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	TSharedPtr<class FUICommandList> PluginCommands;

private:
	TArray<TSharedPtr<FJsonValue>> ImportTable;

private:
	void Main();

	UObject* ResolveImportObject(int32 PackageIndex);

	void SetPropertyValue(FProperty* InProperty, void* PropertyValuePtr, TSharedPtr<FJsonValue> Value);
	void SetObjectProperties(UObject* Object, TSharedPtr<FJsonObject> ObjectProperties);

	/** Utility functions */
	FORCEINLINE UPackage* GetPackage(const FString& PackageName)
	{
		UPackage* Package = FindPackage(nullptr, *PackageName);
		if (!Package)
		{
			Package = LoadPackage(nullptr, *PackageName, LOAD_None);
		}

		return Package;
	}

	FORCEINLINE FString GetAssetName(const FString& PackageName)
	{
		/** Assuming PackageName is in the following format: "/Game/MyFolder/MyAsset" */
		FString AssetName;
		if (PackageName.Split(TEXT("/"), NULL, &AssetName, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
			return AssetName;

		return TEXT("DummyAsset");
	}

	/** Physics Generation */
	UPhysicsAsset* CreatePhysicsAsset(const FString& PhysicsAssetPackageName, TSharedPtr<FJsonObject> PhysicsAssetProperties);
	USkeletalBodySetup* CreateSkeletalBodySetup(TSharedPtr<FJsonObject> BodySetupProperties, UObject* Outer);
	UPhysicsConstraintTemplate* CreateConstraintSetup(TSharedPtr<FJsonObject> ConstraintSetupProperties, UObject* Outer);
};
