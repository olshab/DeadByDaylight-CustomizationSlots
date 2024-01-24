// Copyright Epic Games, Inc. All Rights Reserved.

#include "SlateUITestEdMode.h"
#include "SlateUITestEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "EditorModeManager.h"

const FEditorModeID FSlateUITestEdMode::EM_SlateUITestEdModeId = TEXT("EM_SlateUITestEdMode");

FSlateUITestEdMode::FSlateUITestEdMode()
{

}

FSlateUITestEdMode::~FSlateUITestEdMode()
{

}

void FSlateUITestEdMode::Enter()
{
	FEdMode::Enter();

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FSlateUITestEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}
}

void FSlateUITestEdMode::Exit()
{
	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();
}

bool FSlateUITestEdMode::UsesToolkits() const
{
	return true;
}




