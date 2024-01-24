// Copyright Epic Games, Inc. All Rights Reserved.

#include "SlateUITest.h"
#include "SlateUITestEdMode.h"

#define LOCTEXT_NAMESPACE "FSlateUITestModule"

void FSlateUITestModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FEditorModeRegistry::Get().RegisterMode<FSlateUITestEdMode>(FSlateUITestEdMode::EM_SlateUITestEdModeId, LOCTEXT("SlateUITestEdModeName", "SlateUITestEdMode"), FSlateIcon(), true);
}

void FSlateUITestModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FEditorModeRegistry::Get().UnregisterMode(FSlateUITestEdMode::EM_SlateUITestEdModeId);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSlateUITestModule, SlateUITest)