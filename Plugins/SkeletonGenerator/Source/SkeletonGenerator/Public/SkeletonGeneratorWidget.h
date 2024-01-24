#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

#define LOCTEXT_NAMESPACE "SkeletonGeneratorWidget"

class UGenerationSettings;

class SSkeletonGeneratorWidget : public SCompoundWidget
{
public:
	SSkeletonGeneratorWidget();

	UGenerationSettings* Settings;	

	SLATE_BEGIN_ARGS(SSkeletonGeneratorWidget)
		{}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	FReply OnDetailsViewButtonClicked();
	FReply OnSelectFileButtonClicked();
};

#undef LOCTEXT_NAMESPACE
