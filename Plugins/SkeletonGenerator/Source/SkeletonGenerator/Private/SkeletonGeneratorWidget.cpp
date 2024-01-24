#include "SkeletonGeneratorWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Widgets/Input/SButton.h"
#include "EditorStyleSet.h"
#include "Input/Reply.h"
#include "GenerationSettings.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "SkeletonAssetGenerator.h"

SSkeletonGeneratorWidget::SSkeletonGeneratorWidget()
{
	Settings = UGenerationSettings::Get();
}

void SSkeletonGeneratorWidget::Construct(const FArguments& InArgs)
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;

	TSharedRef<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(Settings);

	ChildSlot
	[
		SNew(SVerticalBox)

		+SVerticalBox::Slot()
		.AutoHeight()
		[
			DetailsView
		]

		+SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Top)
		.Padding(30.0f, 20.0f, 0.0f, 0.0f)
		[
			SNew(SHorizontalBox)

			+SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Left)
			[
				SNew(SButton)
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
				.ContentPadding(FMargin(30, 5))
				.OnClicked(this, &SSkeletonGeneratorWidget::OnDetailsViewButtonClicked)
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Text(FText::FromString(TEXT("Generate")))
				]
			]

			+SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Left)
			.Padding(20.0f, 0.0f)
			[
				SNew(SButton)
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Primary")
				.ContentPadding(FMargin(10, 5))
				.OnClicked(this, &SSkeletonGeneratorWidget::OnSelectFileButtonClicked)
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					.Text(FText::FromString(TEXT("Choose Dump File")))
				]
			]
		]
	];
}

FReply SSkeletonGeneratorWidget::OnDetailsViewButtonClicked()
{
	Settings->SaveConfig(CPF_Config, *Settings->GetDefaultConfigFilename());

	FSkeletonAssetGenerator AssetGenerator(Settings);
	bool Success = AssetGenerator.Generate();

	if (Success)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Successfully generated skeleton")));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Failed to generate skeleton. See the logs")));
	}

	return FReply::Handled();
}

FReply SSkeletonGeneratorWidget::OnSelectFileButtonClicked()
{
	IMainFrameModule& MainFrame = IMainFrameModule::Get();
	TSharedPtr<SWindow> MainWindow = MainFrame.GetParentWindow();
	void* ParentWindowHandle = MainWindow->GetNativeWindow()->GetOSWindowHandle();

	TArray<FString> SelectedFiles;

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	DesktopPlatform->OpenFileDialog(ParentWindowHandle, TEXT("Select Dump File"), TEXT(""), TEXT(""), TEXT("Dumped Skeleton|*.json"), 0, SelectedFiles);

	// TODO: add check if anything selected
	UE_LOG(LogTemp, Log, TEXT("Selected file: %s"), *SelectedFiles[0]);
	Settings->JsonFilePath = SelectedFiles[0];
	Settings->SaveConfig(CPF_Config, *Settings->GetDefaultConfigFilename());

	return FReply::Handled();
}
