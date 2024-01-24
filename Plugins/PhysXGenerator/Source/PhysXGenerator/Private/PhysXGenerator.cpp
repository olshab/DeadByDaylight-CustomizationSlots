// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhysXGenerator.h"
#include "PhysXGeneratorStyle.h"
#include "PhysXGeneratorCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

#include "UObject/Package.h"
#include "UObject/NoExportTypes.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Toolkits/AssetEditorManager.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Factories/PhysicsAssetFactory.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintTemplate.h"

class USkeletalMesh;

static const FName PhysXGeneratorTabName("PhysXGenerator");

#define LOCTEXT_NAMESPACE "FPhysXGeneratorModule"

void FPhysXGeneratorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FPhysXGeneratorStyle::Initialize();
	FPhysXGeneratorStyle::ReloadTextures();

	FPhysXGeneratorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FPhysXGeneratorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FPhysXGeneratorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FPhysXGeneratorModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(PhysXGeneratorTabName, FOnSpawnTab::CreateRaw(this, &FPhysXGeneratorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FPhysXGeneratorTabTitle", "PhysXGenerator"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FPhysXGeneratorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FPhysXGeneratorStyle::Shutdown();

	FPhysXGeneratorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(PhysXGeneratorTabName);
}

TSharedRef<SDockTab> FPhysXGeneratorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FPhysXGeneratorModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("PhysXGenerator.cpp"))
		);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(WidgetText)
			]
		];
}

void FPhysXGeneratorModule::PluginButtonClicked()
{
	// FGlobalTabmanager::Get()->TryInvokeTab(PhysXGeneratorTabName);

	Main();
}

void FPhysXGeneratorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FPhysXGeneratorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FPhysXGeneratorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}


void FPhysXGeneratorModule::Main()
{
	FString JsonFilePath = TEXT("C:\\Users\\Oleg\\Desktop\\UncookedPhysicsAsset.json");

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

	FString AssetName = JsonObject->GetStringField(TEXT("PhysicsAssetAssetName"));
	ImportTable = JsonObject->GetArrayField(TEXT("ImportTable"));

	FString PhysicsAssetPackagePath = TEXT("/Game/PhysX");
	FString PhysicsAssetPackageName = PhysicsAssetPackagePath + TEXT('/') + AssetName;

	TSharedPtr<FJsonObject> PhysicsAssetProperties = JsonObject->GetObjectField(TEXT("PhysicsAssetProperties"));
	if (PhysicsAssetProperties == NULL)
		return;

	/** Creating physics asset */
	UPhysicsAsset* PhysicsAsset = CreatePhysicsAsset(PhysicsAssetPackageName, PhysicsAssetProperties);
	if (PhysicsAsset == NULL)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create physics asset: %s"), *PhysicsAssetPackageName);
		return;
	}

	/** SkeletalBodySetups */
	TArray<TSharedPtr<FJsonValue>> SkeletalBodySetups = JsonObject->GetArrayField(TEXT("SkeletalBodySetups"));
	PhysicsAsset->SkeletalBodySetups.Empty();

	for (TSharedPtr<FJsonValue> SkeletalBodySetupValue : SkeletalBodySetups)
	{
		TSharedPtr<FJsonObject> SkeletalBodySetupProperties = SkeletalBodySetupValue->AsObject();
		if (SkeletalBodySetupProperties == NULL)
			continue;

		USkeletalBodySetup* BodySetup = CreateSkeletalBodySetup(SkeletalBodySetupProperties, PhysicsAsset);
		PhysicsAsset->SkeletalBodySetups.Add(BodySetup);
	}

	/** ConstraintSetup */
	TArray<TSharedPtr<FJsonValue>> ConstraintSetup = JsonObject->GetArrayField(TEXT("ConstraintSetup"));
	PhysicsAsset->ConstraintSetup.Empty();

	for (TSharedPtr<FJsonValue> ConstraintSetupValue : ConstraintSetup)
	{
		TSharedPtr<FJsonObject> ConstraintSetupProperties = ConstraintSetupValue->AsObject();
		if (ConstraintSetupProperties == NULL)
			continue;

		UPhysicsConstraintTemplate* Constraint = CreateConstraintSetup(ConstraintSetupProperties, PhysicsAsset);
		PhysicsAsset->ConstraintSetup.Add(Constraint);
	}
}

UPhysicsAsset* FPhysXGeneratorModule::CreatePhysicsAsset(const FString& PhysicsAssetPackageName, TSharedPtr<FJsonObject> PhysicsAssetProperties)
{
	UPhysicsAssetFactory* Factory = NewObject<UPhysicsAssetFactory>();
	const FString AssetName = GetAssetName(PhysicsAssetPackageName);

	/** Get target SkeletalMesh */
	FString TargetSkeletalMeshPath = TEXT("/Game/PhysX/NK_Hair006_REF.NK_Hair006_REF");
	USkeletalMesh* TargetSkeletalMesh = LoadObject<USkeletalMesh>(GetTransientPackage(), *TargetSkeletalMeshPath);

	UPackage* Package = CreatePackage(*PhysicsAssetPackageName);
	EObjectFlags Flags = RF_Public | RF_Standalone;

	//UObject* CreatedObject = Factory->CreatePhysicsAssetFromMesh(*AssetName, Package, TargetSkeletalMesh, false);
	UPhysicsAsset* CreatedObject = NewObject<UPhysicsAsset>(Package, *AssetName, RF_Public | RF_Standalone);

	if (CreatedObject)
	{
		UPhysicsAsset* PhysicsAsset = CastChecked<UPhysicsAsset>(CreatedObject);
		PhysicsAsset->BoundsBodies.Empty();

		/** Setting the properties for physics asset */
		SetObjectProperties(PhysicsAsset, PhysicsAssetProperties);

		FAssetRegistryModule::AssetCreated(PhysicsAsset);
		PhysicsAsset->MarkPackageDirty();

		return PhysicsAsset;
	}

	return NULL;
}

USkeletalBodySetup* FPhysXGeneratorModule::CreateSkeletalBodySetup(TSharedPtr<FJsonObject> BodySetupProperties, UObject* Outer)
{
	USkeletalBodySetup* BodySetup = NewObject<USkeletalBodySetup>(Outer);
	check(BodySetup);

	SetObjectProperties(BodySetup, BodySetupProperties);

	return BodySetup;
}

UPhysicsConstraintTemplate* FPhysXGeneratorModule::CreateConstraintSetup(TSharedPtr<FJsonObject> ConstraintSetupProperties, UObject* Outer)
{
	UPhysicsConstraintTemplate* ConstraintSetup = NewObject<UPhysicsConstraintTemplate>(Outer);
	check(ConstraintSetup);

	SetObjectProperties(ConstraintSetup, ConstraintSetupProperties);

	return ConstraintSetup;
}

UObject* FPhysXGeneratorModule::ResolveImportObject(int32 PackageIndex)
{
	if (PackageIndex >= 0)
		return nullptr;

	TSharedPtr<FJsonObject> Import = ImportTable[-PackageIndex - 1]->AsObject();

	FString ClassPackageName = Import->GetStringField(TEXT("ClassPackage"));
	FString ClassName = Import->GetStringField(TEXT("ClassName"));
	int32 OuterIndex = Import->GetIntegerField(TEXT("OuterIndex"));
	FString ObjectName = Import->GetStringField(TEXT("ObjectName"));

	/** Imports that have OuterIndex = 0 are just path to the asset */
	if (OuterIndex == 0)
	{
		UPackage* Package = GetPackage(*ObjectName);
		if (!Package)
			UE_LOG(LogTemp, Error, TEXT("Can't deserialize package: %s"), *ObjectName);

		return Package;
	}

	/** Otherwise, go through the entire UObject chain until we find an import with OuterIndex = 0*/
	UPackage* ClassPackage = GetPackage(ClassPackageName);
	if (!ClassPackage)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't deserialize ClassPackage: %s"), *ClassPackageName);
		return nullptr;
	}

	UClass* Class = FindObjectFast<UClass>(ClassPackage, *ClassName);
	if (!Class)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't deserialize ClassName: %s"), *ClassName);
		return nullptr;
	}

	UObject* OuterObject = ResolveImportObject(OuterIndex);
	if (!OuterObject)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't deserialize Outer Object for object: %s"), *ObjectName);
		return nullptr;
	}

	UObject* ResultObject = StaticFindObjectFast(Class, OuterObject, *ObjectName);
	if (!ResultObject)
		UE_LOG(LogTemp, Error, TEXT("Can't deserialize object: %s"), *ObjectName);

	return ResultObject;
}

void FPhysXGeneratorModule::SetPropertyValue(FProperty* InProperty, void* PropertyValuePtr, TSharedPtr<FJsonValue> Value)
{
	if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(InProperty))
	{
		FScriptArrayHelper Array(ArrayProperty, PropertyValuePtr);

		Array.EmptyValues();

		const TArray<TSharedPtr<FJsonValue>>& JsonArrayValues = Value->AsArray();
		for (const TSharedPtr<FJsonValue>& JsonArrayValue : JsonArrayValues)
		{
			int32 NewIndex = Array.AddValue();
			uint8* ArrayValuePtr = Array.GetRawPtr(NewIndex);
			SetPropertyValue(ArrayProperty->Inner, ArrayValuePtr, JsonArrayValue);
		}

		return;
	}

	if (FStructProperty* StructProperty = CastField<FStructProperty>(InProperty))
	{
		UScriptStruct* Struct = StructProperty->Struct;

		TArray<FString> JsonProperties;
		Value->AsObject()->Values.GetKeys(JsonProperties);

		for (FString& JsonProperty : JsonProperties)
		{
			FProperty* Property = Struct->FindPropertyByName(*JsonProperty);
			if (Property == nullptr)
			{
				UE_LOG(LogTemp, Error, TEXT("Can't find property %s from JSON in ScriptStruct"), *JsonProperty);
				continue;
			}

			SetPropertyValue(
				Property,
				Property->ContainerPtrToValuePtr<void>(PropertyValuePtr),
				Value->AsObject()->GetField<EJson::None>(JsonProperty)
			);
		}

		return;
	}

	if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(InProperty))
	{
		BoolProperty->SetPropertyValue(PropertyValuePtr, Value->AsBool());

		return;
	}

	if (FByteProperty* ByteProperty = CastField<FByteProperty>(InProperty))
	{
		UEnum* IntPropertyEnum = ByteProperty->GetIntPropertyEnum();
		if (IntPropertyEnum)
		{
			FString EnumValue = Value->AsString();
			ByteProperty->SetIntPropertyValue(PropertyValuePtr, IntPropertyEnum->GetValueByNameString(EnumValue));
		}
		else
		{
			int64 ByteValue = (int64)Value->AsNumber();
			ByteProperty->SetIntPropertyValue(PropertyValuePtr, ByteValue);
		}

		return;
	}

	if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(InProperty))
	{
		FNumericProperty* UnderlyingProperty = EnumProperty->GetUnderlyingProperty();
		UEnum* Enum = EnumProperty->GetEnum();

		FString EnumValue = Value->AsString();
		UnderlyingProperty->SetIntPropertyValue(PropertyValuePtr, Enum->GetValueByNameString(EnumValue));

		return;
	}

	if (FNumericProperty* NumericProperty = CastField<FNumericProperty>(InProperty))
	{
		if (NumericProperty->IsFloatingPoint())
			NumericProperty->SetFloatingPointPropertyValue(PropertyValuePtr, Value->AsNumber());
		else
			NumericProperty->SetIntPropertyValue(PropertyValuePtr, (int64)Value->AsNumber());

		return;
	}

	if (FNameProperty* NameProperty = CastField<FNameProperty>(InProperty))
	{
		FName* NamePtr = static_cast<FName*>(PropertyValuePtr);
		*NamePtr = *Value->AsString();

		return;
	}

	if (FStrProperty* StrProperty = CastField<FStrProperty>(InProperty))
	{
		FString* StrPtr = static_cast<FString*>(PropertyValuePtr);
		*StrPtr = *Value->AsString();

		return;
	}

	if (FTextProperty* TextProperty = CastField<FTextProperty>(InProperty))
	{
		FText* TextPtr = static_cast<FText*>(PropertyValuePtr);
		*TextPtr = FText::FromString(Value->AsString());

		return;
	}

	if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(InProperty))
	{
		int32 PackageIndex = (int32)Value->AsNumber();
		UObject* Object = NULL;

		if (PackageIndex < 0)
		{
			Object = ResolveImportObject(PackageIndex);

			if (Object == NULL)
				UE_LOG(LogTemp, Warning, TEXT("Failed to load object for property %s"), *InProperty->GetName());
		}

		ObjectProperty->SetObjectPropertyValue(PropertyValuePtr, Object);

		return;
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to resolve FProperty type for property %s"), *InProperty->GetName())
}

void FPhysXGeneratorModule::SetObjectProperties(UObject* Object, TSharedPtr<FJsonObject> ObjectProperties)
{
	TArray<FString> PropertyNames;
	ObjectProperties->Values.GetKeys(PropertyNames);

	for (const FString& JsonProperty : PropertyNames)
	{
		FProperty* Property = Object->GetClass()->FindPropertyByName(*JsonProperty);
		if (!Property)
		{
			UE_LOG(LogTemp, Error, TEXT("%s doesn't have property with the name %s"), *Object->GetName(), *JsonProperty);
			continue;
		}

		SetPropertyValue(
			Property,
			Property->ContainerPtrToValuePtr<void>(Object),
			ObjectProperties->GetField<EJson::None>(JsonProperty)
		);
	}
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPhysXGeneratorModule, PhysXGenerator)