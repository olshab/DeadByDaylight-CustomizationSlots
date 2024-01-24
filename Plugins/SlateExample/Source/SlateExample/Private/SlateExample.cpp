// Copyright Epic Games, Inc. All Rights Reserved.

#include "SlateExample.h"
#include "SlateExampleStyle.h"
#include "SlateExampleCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "TestWidget.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "TestSettings.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "EditorStyleSet.h"
#include "Widgets/Text/STextBlock.h"

static const FName SlateExampleTabName("SlateExample");

#define LOCTEXT_NAMESPACE "FSlateExampleModule"

void FSlateExampleModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FSlateExampleStyle::Initialize();
	FSlateExampleStyle::ReloadTextures();

	FSlateExampleCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FSlateExampleCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FSlateExampleModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FSlateExampleModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(SlateExampleTabName, FOnSpawnTab::CreateRaw(this, &FSlateExampleModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FSlateExampleTabTitle", "SlateExample"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FSlateExampleModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FSlateExampleStyle::Shutdown();

	FSlateExampleCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SlateExampleTabName);
}

TSharedRef<SDockTab> FSlateExampleModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs(false, false, true, FDetailsViewArgs::HideNameArea, true, this);

	TSharedRef<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(UTestSettings::Get());

	return 
		SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(STestWidget)
		];
}

void FSlateExampleModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(SlateExampleTabName);
}

void FSlateExampleModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FSlateExampleCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FSlateExampleCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSlateExampleModule, SlateExample)