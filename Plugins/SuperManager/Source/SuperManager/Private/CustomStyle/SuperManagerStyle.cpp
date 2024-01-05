// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomStyle/SuperManagerStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

FName FSuperManagerStyle::StyleSetName = FName("SuperManagerStyle");
TSharedPtr<FSlateStyleSet> FSuperManagerStyle::CreatedSlateStyleSet = nullptr;

void FSuperManagerStyle::InitIcons()
{
	if (CreatedSlateStyleSet.IsValid() == false)
	{
		CreatedSlateStyleSet = CreateSlateStyleSet();
		FSlateStyleRegistry::RegisterSlateStyle(*CreatedSlateStyleSet);
	}
}

void FSuperManagerStyle::ShutDown()
{
	if (CreatedSlateStyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*CreatedSlateStyleSet);
		CreatedSlateStyleSet.Reset();
	}
}

TSharedRef<FSlateStyleSet> FSuperManagerStyle::CreateSlateStyleSet()
{
	TSharedRef<FSlateStyleSet> CustomSlateStyleSet = MakeShareable(new FSlateStyleSet(StyleSetName));

	FString IconsDir = IPluginManager::Get().FindPlugin(TEXT("SuperManager"))->GetBaseDir() / "Resources";
	CustomSlateStyleSet->SetContentRoot(IconsDir);

	const FVector2D Icon16x16(16.f, 16.f);

	CustomSlateStyleSet->Set("ContentBrowser.DeleteUnusedAssets", new FSlateImageBrush(IconsDir / "DeleteUnusedAsset.png", Icon16x16));
	CustomSlateStyleSet->Set("ContentBrowser.DeleteEmptyFolders", new FSlateImageBrush(IconsDir / "DeleteEmptyFolders.png", Icon16x16));
	CustomSlateStyleSet->Set("ContentBrowser.AdvanceDeletion", new FSlateImageBrush(IconsDir / "AdvanceDeletion.png", Icon16x16));
	CustomSlateStyleSet->Set("ContentBrowser.DeleteEmptyFoldersAndUnusedAssets", new FSlateImageBrush(IconsDir / "Icon128.png", Icon16x16));

	return CustomSlateStyleSet;
}
