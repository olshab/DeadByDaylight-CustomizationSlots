// Copyright Epic Games, Inc. All Rights Reserved.

#include "SkeletonGeneratorCommands.h"

#define LOCTEXT_NAMESPACE "FSkeletonGeneratorModule"

void FSkeletonGeneratorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "SkeletonGenerator", "Execute SkeletonGenerator action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
