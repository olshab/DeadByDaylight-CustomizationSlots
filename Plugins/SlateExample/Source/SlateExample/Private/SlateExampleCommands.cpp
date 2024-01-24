// Copyright Epic Games, Inc. All Rights Reserved.

#include "SlateExampleCommands.h"

#define LOCTEXT_NAMESPACE "FSlateExampleModule"

void FSlateExampleCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "SlateExample", "Bring up SlateExample window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
