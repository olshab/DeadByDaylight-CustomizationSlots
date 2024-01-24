// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "PhysXGeneratorStyle.h"

class FPhysXGeneratorCommands : public TCommands<FPhysXGeneratorCommands>
{
public:

	FPhysXGeneratorCommands()
		: TCommands<FPhysXGeneratorCommands>(TEXT("PhysXGenerator"), NSLOCTEXT("Contexts", "PhysXGenerator", "PhysXGenerator Plugin"), NAME_None, FPhysXGeneratorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};