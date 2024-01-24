// Copyright Epic Games, Inc. All Rights Reserved.

#include "SkeletonGenerator.h"
#include "SkeletonGeneratorStyle.h"
#include "SkeletonGeneratorCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMeshSocket.h"
#include "SkeletonGeneratorWidget.h"

static const FName SkeletonGeneratorTabName("SkeletonGenerator");

#define LOCTEXT_NAMESPACE "FSkeletonGeneratorModule"

void FSkeletonGeneratorModule::StartupModule()
{
	FSkeletonGeneratorStyle::Initialize();
	FSkeletonGeneratorStyle::ReloadTextures();

	FSkeletonGeneratorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FSkeletonGeneratorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FSkeletonGeneratorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FSkeletonGeneratorModule::RegisterMenus));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(SkeletonGeneratorTabName, FOnSpawnTab::CreateRaw(this, &FSkeletonGeneratorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FSkeletonGeneratorTabTitle", "Skeleton Generator"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FSkeletonGeneratorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FSkeletonGeneratorStyle::Shutdown();
	FSkeletonGeneratorCommands::Unregister();
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SkeletonGeneratorTabName);
}

TSharedRef<SDockTab> FSkeletonGeneratorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return
		SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SSkeletonGeneratorWidget)
		];
}

void FSkeletonGeneratorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(SkeletonGeneratorTabName);
}

void FSkeletonGeneratorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FSkeletonGeneratorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FSkeletonGeneratorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}


void FSkeletonGeneratorModule::Main()
{
	FString JsonFilePath = TEXT("C:\\Users\\Oleg\\Desktop\\Skeleton.json");

	UPackage* SkeletonPackage = CreatePackage(TEXT("/Game/UI/UncookedSkeleton"));
	USkeleton* Skeleton = NewObject<USkeleton>(SkeletonPackage, TEXT("UncookedSkeleton"), RF_Public | RF_Standalone);

	TArray<FBoneNode>& BoneTree = *(TArray<FBoneNode>*)((uint8*)Skeleton + 0x40);
	FReferenceSkeleton* ReferenceSkeleton = (FReferenceSkeleton*)((uint8*)Skeleton + 0x60);

	TArray<FMeshBoneInfo>& RawRefBoneInfo = *(TArray<FMeshBoneInfo>*)((uint8*)ReferenceSkeleton + 0x00);
	TArray<FTransform>& RawRefBonePose = *(TArray<FTransform>*)((uint8*)ReferenceSkeleton + 0x10);
	TArray<FMeshBoneInfo>& FinalRefBoneInfo = *(TArray<FMeshBoneInfo>*)((uint8*)ReferenceSkeleton + 0x20);
	TArray<FTransform>& FinalRefBonePose = *(TArray<FTransform>*)((uint8*)ReferenceSkeleton + 0x30);
	TMap<FName, int32>& RawNameToIndexMap = *(TMap<FName, int32>*)((uint8*)ReferenceSkeleton + 0x40);
	TMap<FName, int32>& FinalNameToIndexMap = *(TMap<FName, int32>*)((uint8*)ReferenceSkeleton + 0x90);

	FGuid& Guid = *(FGuid*)((uint8*)Skeleton + 0x0168);

	TArray<FAnimSlotGroup>& SlotGroups = *(TArray<FAnimSlotGroup>*)((uint8*)Skeleton + 0x02D8);

	if (JsonFilePath.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get dump file path from config"));
		return;
	}

	FString JsonContents;
	if (!FFileHelper::LoadFileToString(JsonContents, *JsonFilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Can't load string from file %s"), *JsonFilePath);
		return;
	}

	TSharedPtr<FJsonObject> JsonObject;
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(JsonContents), JsonObject))
	{
		UE_LOG(LogTemp, Error, TEXT("Can't deserialize Json file: %s"), *JsonFilePath);
		return;
	}

	/** BoneTree */
	const TArray<TSharedPtr<FJsonValue>>& BoneTreeJson = JsonObject->GetArrayField(TEXT("BoneTree"));
	BoneTree.Empty();

	for (const TSharedPtr<FJsonValue>& BoneTreeElem : BoneTreeJson)
	{
		uint8 Mode = BoneTreeElem->AsNumber();
		int32 InsertedIndex = BoneTree.Add(FBoneNode());
		BoneTree[InsertedIndex].TranslationRetargetingMode = TEnumAsByte<EBoneTranslationRetargetingMode::Type>(Mode);
	}

	/** ReferenceSkeleton */
	const TSharedPtr<FJsonObject> ReferenceSkeletonJson = JsonObject->GetObjectField(TEXT("ReferenceSkeleton"));
	ReferenceSkeleton->Empty();

	/** ReferenceSkeleton.RawRefBoneInfo */
	const TArray<TSharedPtr<FJsonValue>>& RawRefBoneInfoJson = ReferenceSkeletonJson->GetArrayField(TEXT("RawRefBoneInfo"));
	
	for (const TSharedPtr<FJsonValue>& BoneInfoElem : RawRefBoneInfoJson)
	{
		const TSharedPtr<FJsonObject>& BoneInfoJson = BoneInfoElem->AsObject();

		FString Name = BoneInfoJson->GetStringField(TEXT("Name"));
		int32 ParentIndex = BoneInfoJson->GetNumberField(TEXT("ParentIndex"));
		FString ExportName = BoneInfoJson->GetStringField(TEXT("ExportName"));

		RawRefBoneInfo.Add(FMeshBoneInfo(*Name, ExportName, ParentIndex));
		FinalRefBoneInfo.Add(FMeshBoneInfo(*Name, ExportName, ParentIndex));
	}

	/** ReferenceSkeleton.RawRefBonePose */
	const TArray<TSharedPtr<FJsonValue>>& RawRefBonePoseJson = ReferenceSkeletonJson->GetArrayField(TEXT("RawRefBonePose"));

	for (const TSharedPtr<FJsonValue>& BonePoseElem : RawRefBonePoseJson)
	{
		const TSharedPtr<FJsonObject>& TransformJson = BonePoseElem->AsObject();

		const TSharedPtr<FJsonObject>& TranslationJson = TransformJson->GetObjectField(TEXT("Translation"));
		FVector Translation = FVector(
			TranslationJson->GetNumberField(TEXT("X")),
			TranslationJson->GetNumberField(TEXT("Y")),
			TranslationJson->GetNumberField(TEXT("Z"))
		);

		const TSharedPtr<FJsonObject>& RotationJson = TransformJson->GetObjectField(TEXT("Rotation"));
		FQuat Rotation = FQuat(
			RotationJson->GetNumberField(TEXT("X")),
			RotationJson->GetNumberField(TEXT("Y")),
			RotationJson->GetNumberField(TEXT("Z")),
			RotationJson->GetNumberField(TEXT("W"))
		);

		const TSharedPtr<FJsonObject>& ScaleJson = TransformJson->GetObjectField(TEXT("Scale"));
		FVector Scale = FVector(
			ScaleJson->GetNumberField(TEXT("X")),
			ScaleJson->GetNumberField(TEXT("Y")),
			ScaleJson->GetNumberField(TEXT("Z"))
		);

		RawRefBonePose.Add(FTransform(Rotation, Translation, Scale));
		FinalRefBonePose.Add(FTransform(Rotation, Translation, Scale));
	}

	const TArray<TSharedPtr<FJsonValue>>& RawNameToIndexMapJson = ReferenceSkeletonJson->GetArrayField(TEXT("RawNameToIndexMap"));

	for (const TSharedPtr<FJsonValue>& NameToIndexElem : RawNameToIndexMapJson)
	{
		const TSharedPtr<FJsonObject>& KeyValuePair = NameToIndexElem->AsObject();

		FString Key = KeyValuePair->GetStringField(TEXT("Key"));
		int32 Value = KeyValuePair->GetNumberField(TEXT("Value"));

		RawNameToIndexMap.Add(*Key, Value);
		FinalNameToIndexMap.Add(*Key, Value);
	}

	FString GuidString = JsonObject->GetStringField(TEXT("Guid"));
	Guid = FGuid(GuidString);

	/** SlotGroups */
	SlotGroups.Empty();

	const TArray<TSharedPtr<FJsonValue>>& SlotGroupsArray = JsonObject->GetArrayField(TEXT("SlotGroups"));

	for (const TSharedPtr<FJsonValue>& SlotGroupJson : SlotGroupsArray)
	{
		const TSharedPtr<FJsonObject>& SlotGroupObject = SlotGroupJson->AsObject();

		FString GroupName = SlotGroupObject->GetStringField(TEXT("GroupName"));
		int32 AddedIndex = SlotGroups.Add(FName(*GroupName));
		
		const TArray<TSharedPtr<FJsonValue>>& SlotNamesArray = SlotGroupObject->GetArrayField(TEXT("SlotNames"));
		for (const TSharedPtr<FJsonValue>& SlotNamesJson : SlotNamesArray)
			SlotGroups[AddedIndex].SlotNames.Add(*SlotNamesJson->AsString());
	}

	/** Sockets */
	Skeleton->Sockets.Empty();

	const TArray<TSharedPtr<FJsonValue>>& SocketsArray = JsonObject->GetArrayField(TEXT("Sockets"));

	for (const TSharedPtr<FJsonValue>& SocketJson : SocketsArray)
	{
		const TSharedPtr<FJsonObject>& SocketObject = SocketJson->AsObject();

		FString SocketName = SocketObject->GetStringField(TEXT("SocketName"));
		FString BoneName = SocketObject->GetStringField(TEXT("BoneName"));

		const TSharedPtr<FJsonObject>& LocationJson = SocketObject->GetObjectField(TEXT("RelativeLocation"));
		FVector RelativeLocation = FVector(
			LocationJson->GetNumberField(TEXT("X")),
			LocationJson->GetNumberField(TEXT("Y")),
			LocationJson->GetNumberField(TEXT("Z"))
		);

		const TSharedPtr<FJsonObject>& RotationJson = SocketObject->GetObjectField(TEXT("RelativeRotation"));
		FRotator RelativeRotation = FRotator(
			RotationJson->GetNumberField(TEXT("Pitch")),
			RotationJson->GetNumberField(TEXT("Yaw")),
			RotationJson->GetNumberField(TEXT("Roll"))
		);

		const TSharedPtr<FJsonObject>& ScaleJson = SocketObject->GetObjectField(TEXT("RelativeScale"));
		FVector RelativeScale = FVector(
			ScaleJson->GetNumberField(TEXT("X")),
			ScaleJson->GetNumberField(TEXT("Y")),
			ScaleJson->GetNumberField(TEXT("Z"))
		);

		USkeletalMeshSocket* SkeletalMeshSocket = NewObject<USkeletalMeshSocket>(Skeleton);
		check(SkeletalMeshSocket);

		SkeletalMeshSocket->SocketName = *SocketName;
		SkeletalMeshSocket->BoneName = *BoneName;
		SkeletalMeshSocket->RelativeLocation = RelativeLocation;
		SkeletalMeshSocket->RelativeRotation = RelativeRotation;
		SkeletalMeshSocket->RelativeScale = RelativeScale;

		Skeleton->Sockets.Add(SkeletalMeshSocket);
	}

	Skeleton->MarkPackageDirty();
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSkeletonGeneratorModule, SkeletonGenerator)