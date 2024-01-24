#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "TestSettings.h"
#include "Widgets/Input/SButton.h"
#include "EditorStyleSet.h"
#include "Widgets/Text/STextBlock.h"
#include "Input/Reply.h"

#define LOCTEXT_NAMESPACE "TestWidget"

class STestWidget : public SCompoundWidget
{
public:
	UTestSettings* Settings;

	STestWidget()
	{
		Settings = UTestSettings::Get();
	}

	SLATE_BEGIN_ARGS(STestWidget)
	{}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		FDetailsViewArgs DetailsViewArgs;

		TSharedRef<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
		DetailsView->SetObject(Settings);

		ChildSlot
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			[
				DetailsView
			]

			+ SVerticalBox::Slot()
			[
				SNew(SButton)
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Primary")
				.ContentPadding(FMargin(10, 0))
				.ToolTipText(FText::FromString(TEXT("Press Me!")))
				.OnClicked(this, &STestWidget::OnDetailsViewButtonClicked)
				[
					SNew(STextBlock)
						.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
						.Text(FText::FromString(TEXT("Press Right Now")))
				]
			]
		];
	}

	FReply OnDetailsViewButtonClicked()
	{
		UE_LOG(LogTemp, Log, TEXT("Details View Button clicked"));
		UE_LOG(LogTemp, Log, TEXT("Settings config filename: %s"), *Settings->GetDefaultConfigFilename());
		Settings->SaveConfig(CPF_Config, *Settings->GetDefaultConfigFilename());

		UE_LOG(LogTemp, Log, TEXT("Asset Path: %s"), *Settings->Mesh.ToString());
		UObject* Object = Settings->Mesh.TryLoad();

		if (Object)
		{
			UE_LOG(LogTemp, Log, TEXT("Object Loaded: %s"), *Object->GetFullName());
		}

		return FReply::Handled();
	}

	void OnTextCommitted(const FText& NewText, ETextCommit::Type)
	{
		UE_LOG(LogTemp, Log, TEXT("Text Committed: %s"), *NewText.ToString());
	}
};

#undef LOCTEXT_NAMESPACE
