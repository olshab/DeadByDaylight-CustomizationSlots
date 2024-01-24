#include "SkeletonAssetGenerator.h"
#include "GenerationSettings.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMeshSocket.h"
#include "SkeletonGeneratorWidget.h"

bool FSkeletonAssetGenerator::Generate()
{
	const FString& JsonFilePath = Settings->JsonFilePath;

	FString JsonContents;
	if (!FFileHelper::LoadFileToString(JsonContents, *JsonFilePath))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Can't load string from file. See the log")));

		UE_LOG(LogTemp, Error, TEXT("Can't load string from file %s"), *JsonFilePath);
		return false;
	}

	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(JsonContents), AssetJsonObject))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Can't deserialize Json file. See the log")));

		UE_LOG(LogTemp, Error, TEXT("Can't deserialize Json file: %s"), *JsonFilePath);
		return false;
	}

	FString AssetName = Settings->AssetName;
	if (Settings->bUseAssociatedMesh && Settings->bUseDefaultAssetName)
	{
		AssetName = Settings->AssociatedMesh.GetAssetName() + TEXT("_Skeleton");
	}

	USkeleton* CreatedSkeleton = CreateSkeletonObject(Settings->GenerationFolder, AssetName);
	if (CreatedSkeleton == NULL)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Failed to create skeleton. See the log")));

		UE_LOG(LogTemp, Error, TEXT("Failed to create skeleton at path: %s"), *Settings->GenerationFolder);
		return false;
	}

	Skeleton = CreatedSkeleton;

	GenerateBoneTree();
	GenerateReferenceSkeleton();
	GenerateGuid();
	if (Settings->bGenerateSlots) GenerateSlotGroups();
	if (Settings->bGenerateSockets) GenerateSockets();

	if (Settings->bUseAssociatedMesh)
	{
		Skeleton->SetPreviewMesh(CastChecked<USkeletalMesh>(Settings->AssociatedMesh.TryLoad()));
	}

	Skeleton->MarkPackageDirty();
	
	return true;
}

USkeleton* FSkeletonAssetGenerator::CreateSkeletonObject(const FString& PackagePath, const FString& AssetName)
{
	const FString PackageName = PackagePath + TEXT("/") + AssetName;

	UPackage* SkeletonPackage = CreatePackage(*PackageName);
	USkeleton* CreatedSkeleton = NewObject<USkeleton>(SkeletonPackage, *AssetName, RF_Public | RF_Standalone);

	return CreatedSkeleton;
}

void FSkeletonAssetGenerator::GenerateBoneTree()
{
	TArray<FBoneNode>& BoneTree = *(TArray<FBoneNode>*)((uint8*)Skeleton + 0x40);

	const TArray<TSharedPtr<FJsonValue>>& BoneTreeJson = AssetJsonObject->GetArrayField(TEXT("BoneTree"));
	BoneTree.Empty();

	for (const TSharedPtr<FJsonValue>& BoneTreeElem : BoneTreeJson)
	{
		uint8 Mode = BoneTreeElem->AsNumber();
		int32 InsertedIndex = BoneTree.Add(FBoneNode());
		BoneTree[InsertedIndex].TranslationRetargetingMode = TEnumAsByte<EBoneTranslationRetargetingMode::Type>(Mode);
	}
}

void FSkeletonAssetGenerator::GenerateReferenceSkeleton()
{
	FReferenceSkeleton* ReferenceSkeleton = (FReferenceSkeleton*)((uint8*)Skeleton + 0x60);

	TArray<FMeshBoneInfo>& RawRefBoneInfo = *(TArray<FMeshBoneInfo>*)((uint8*)ReferenceSkeleton + 0x00);
	TArray<FTransform>& RawRefBonePose = *(TArray<FTransform>*)((uint8*)ReferenceSkeleton + 0x10);
	TArray<FMeshBoneInfo>& FinalRefBoneInfo = *(TArray<FMeshBoneInfo>*)((uint8*)ReferenceSkeleton + 0x20);
	TArray<FTransform>& FinalRefBonePose = *(TArray<FTransform>*)((uint8*)ReferenceSkeleton + 0x30);
	TMap<FName, int32>& RawNameToIndexMap = *(TMap<FName, int32>*)((uint8*)ReferenceSkeleton + 0x40);
	TMap<FName, int32>& FinalNameToIndexMap = *(TMap<FName, int32>*)((uint8*)ReferenceSkeleton + 0x90);

	const TSharedPtr<FJsonObject> ReferenceSkeletonJson = AssetJsonObject->GetObjectField(TEXT("ReferenceSkeleton"));
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
}

void FSkeletonAssetGenerator::GenerateGuid()
{
	FGuid& Guid = *(FGuid*)((uint8*)Skeleton + 0x0168);

	FString GuidString = AssetJsonObject->GetStringField(TEXT("Guid"));
	Guid = FGuid(GuidString);
}

void FSkeletonAssetGenerator::GenerateSlotGroups()
{
	TArray<FAnimSlotGroup>& SlotGroups = *(TArray<FAnimSlotGroup>*)((uint8*)Skeleton + 0x02D8);

	SlotGroups.Empty();

	const TArray<TSharedPtr<FJsonValue>>& SlotGroupsArray = AssetJsonObject->GetArrayField(TEXT("SlotGroups"));

	for (const TSharedPtr<FJsonValue>& SlotGroupJson : SlotGroupsArray)
	{
		const TSharedPtr<FJsonObject>& SlotGroupObject = SlotGroupJson->AsObject();

		FString GroupName = SlotGroupObject->GetStringField(TEXT("GroupName"));
		int32 AddedIndex = SlotGroups.Add(FName(*GroupName));

		const TArray<TSharedPtr<FJsonValue>>& SlotNamesArray = SlotGroupObject->GetArrayField(TEXT("SlotNames"));
		for (const TSharedPtr<FJsonValue>& SlotNamesJson : SlotNamesArray)
			SlotGroups[AddedIndex].SlotNames.Add(*SlotNamesJson->AsString());
	}
}

void FSkeletonAssetGenerator::GenerateSockets()
{
	Skeleton->Sockets.Empty();

	const TArray<TSharedPtr<FJsonValue>>& SocketsArray = AssetJsonObject->GetArrayField(TEXT("Sockets"));

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
}
