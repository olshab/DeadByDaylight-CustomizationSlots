// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhysXGeneratorCommands.h"

#define LOCTEXT_NAMESPACE "FPhysXGeneratorModule"

void FPhysXGeneratorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "PhysXGenerator", "Bring up PhysXGenerator window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
