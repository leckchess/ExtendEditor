// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FSuperManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;


#pragma region ContentBrowserMenuExtention
private:

	void InitCBMenuExtention();
	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);
	void AddCBMenuEntry(class FMenuBuilder& MenuBuilder);
	void OnDeleteUnusedAssetButtonCLicked();
	void OnDeleteEmptyFoldersButtonCLicked();
	void OnDeleteUnusedAssetsAndEmptyFoldersButtonCLicked();
	void OnAdvancedDeletionButtonCLicked();
	void FixupRedirectors();

private:
	TArray<FString> SelectedFolderPath;
#pragma endregion

#pragma region CustomEditorTab
private:
	void RegisterAdvancedDeletionTab();

	TSharedRef<SDockTab> OnSpawnAdvancedDeletionTab(const FSpawnTabArgs& SpawnTabArgs);

	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderSelectedFolder();

#pragma endregion

#pragma region ProcessDataForAdvancedDeletionWidget
public:
	bool DeleteSingleAsset(const FAssetData& AssetDataToDelete);
	int32 DeleteMultipleAssets(const TArray<FAssetData>& AssetDataToDeleteArray);
	void ListUnusedAssets(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutUnusedAssetData);
	void ListSameNameAssets(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutSameNameAssetData);

#pragma endregion



};
