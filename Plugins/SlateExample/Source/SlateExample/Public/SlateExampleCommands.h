// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "SlateExampleStyle.h"

class FSlateExampleCommands : public TCommands<FSlateExampleCommands>
{
public:

	FSlateExampleCommands()
		: TCommands<FSlateExampleCommands>(TEXT("SlateExample"), NSLOCTEXT("Contexts", "SlateExample", "SlateExample Plugin"), NAME_None, FSlateExampleStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};