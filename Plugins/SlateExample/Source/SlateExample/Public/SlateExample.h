// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Input/Reply.h"

class FToolBarBuilder;
class FMenuBuilder;

class FSlateExampleModule : public IModuleInterface, public FNotifyHook, public TSharedFromThis<FSlateExampleModule>
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

public:
	FReply OnDetailsViewButtonClicked();

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};
