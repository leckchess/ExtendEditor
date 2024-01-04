// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"
#include "ContentBrowserModule.h"
#include "DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetToolsModule.h"
#include "AssetViewUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "SlateWidgets/AdvancedDeletionWidget.h"

#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerModule::StartupModule()
{
	InitCBMenuExtention();
	RegisterAdvancedDeletionTab();
}

void FSuperManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#pragma region ContentBrowserMenuExtention
void FSuperManagerModule::InitCBMenuExtention()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders = ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	//FContentBrowserMenuExtender_SelectedPaths CustonCBMenuDelegate;
	//CustonCBMenuDelegate.BindRaw(this, &FSuperManagerModule::CustomCBMenuExtender);
	//ContentBrowserModuleMenuExtenders.Add(CustonCBMenuDelegate);

	ContentBrowserModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FSuperManagerModule::CustomCBMenuExtender));
}

TSharedRef<FExtender> FSuperManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());

	if (SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(FName("Delete"), EExtensionHook::After, TSharedPtr<FUICommandList>(), FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddCBMenuEntry));

		SelectedFolderPath = SelectedPaths;
	}

	return MenuExtender;
}

void FSuperManagerModule::AddCBMenuEntry(class FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete unused assets")),
		FText::FromString(TEXT("Safely delete all unused assets under folder")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnusedAssetButtonCLicked)
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete empty folders")),
		FText::FromString(TEXT("Safely delete all empty folders")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteEmptyFoldersButtonCLicked)
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete empty folders and unused assets")),
		FText::FromString(TEXT("Safely delete all empty folders and unused assets")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnusedAssetsAndEmptyFoldersButtonCLicked)
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Advanced Deletion")),
		FText::FromString(TEXT("List assets by specific conditions in a tab for deleting")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnAdvancedDeletionButtonCLicked)
	);
}

void FSuperManagerModule::OnDeleteUnusedAssetButtonCLicked()
{
	if (SelectedFolderPath.Num() > 1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("You can only do this to one folder"));
		return;
	}

	TArray<FString> AssetsNames = UEditorAssetLibrary::ListAssets(SelectedFolderPath[0]);

	if (AssetsNames.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No assets found under selected folder"));
		return;
	}

	EAppReturnType::Type ReturnResult = DebugHeader::ShowMsgDialog(EAppMsgType::YesNo, TEXT("A total of ") + FString::FromInt(AssetsNames.Num()) + " assets found\nWould you like to proceed", false);

	if (ReturnResult == EAppReturnType::No)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Operation Canceled"));
		return;
	}

	FixupRedirectors();

	TArray<FAssetData> UnusedAssetsData;

	for (const FString& AssetName : AssetsNames)
	{
		if (AssetName.Contains(TEXT("Collections")) || AssetName.Contains(TEXT("Developers"))) { continue; }

		if (UEditorAssetLibrary::DoesAssetExist(AssetName) == false) { continue; }

		TArray<FString> AssetsReferences = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetName);

		if (AssetsReferences.Num() == 0)
		{
			UnusedAssetsData.Add(UEditorAssetLibrary::FindAssetData(AssetName));
		}
	}

	if (UnusedAssetsData.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused assets found under selected folder"), false);
		return;
	}

	int32 NumOfAssetsDeleted = ObjectTools::DeleteAssets(UnusedAssetsData);

	if (NumOfAssetsDeleted == 0) { return; }

	DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted " + FString::FromInt(NumOfAssetsDeleted) + " unused assets"));
}

void FSuperManagerModule::OnDeleteEmptyFoldersButtonCLicked()
{
	if (SelectedFolderPath.Num() > 1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("You can only do this to one folder"));
		return;
	}

	TArray<FString> AssetsNames = UEditorAssetLibrary::ListAssets(SelectedFolderPath[0], true, true);

	if (AssetsNames.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No folders found under selected folder"));
		return;
	}

	FixupRedirectors();

	FString EmptyFoldersPathNames;
	TArray<FString> EmptyFolders;
	int32 NumberOfFolders = 0;

	for (const FString& AssetName : AssetsNames)
	{
		if (AssetName.Contains(TEXT("Collections")) || AssetName.Contains(TEXT("Developers"))) { continue; }

		if (UEditorAssetLibrary::DoesDirectoryExist(AssetName))
		{
			NumberOfFolders++;

			if (UEditorAssetLibrary::DoesDirectoryHaveAssets(AssetName)) { continue; }

			EmptyFolders.Add(AssetName);

			EmptyFoldersPathNames.Append(AssetName);
			EmptyFoldersPathNames.Append(TEXT("\n"));
		}
	}

	EAppReturnType::Type ReturnResult = DebugHeader::ShowMsgDialog(EAppMsgType::YesNo, TEXT("A total of ") + FString::FromInt(NumberOfFolders) + " folders found\nWould you like to proceed", false);

	if (ReturnResult == EAppReturnType::No)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Operation Canceled"));
		return;
	}

	if (EmptyFolders.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No empty folders found under selected folder"), false);
		return;
	}

	ReturnResult = DebugHeader::ShowMsgDialog(EAppMsgType::OkCancel, TEXT("Empty folder found: \n") + EmptyFoldersPathNames + "\nWould you like to delete all");

	if (ReturnResult == EAppReturnType::Cancel)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Operation Canceled"));
		return;
	}

	int32 NumOfDeletedFolders = 0;

	for (const FString& FolderName : EmptyFolders)
	{
		if (UEditorAssetLibrary::DeleteDirectory(FolderName))
		{
			NumOfDeletedFolders++;
			continue;
		}

		DebugHeader::Print(TEXT("Failed to delete " + FolderName), FColor::Red);
	}

	if (NumOfDeletedFolders == 0) { return; }

	DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted " + FString::FromInt(NumOfDeletedFolders) + " empty folders"));

}

void FSuperManagerModule::OnDeleteUnusedAssetsAndEmptyFoldersButtonCLicked()
{
	OnDeleteUnusedAssetButtonCLicked();
	OnDeleteEmptyFoldersButtonCLicked();
}

void FSuperManagerModule::OnAdvancedDeletionButtonCLicked()
{
	FixupRedirectors();

	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvancedDeletion"));
}

void FSuperManagerModule::FixupRedirectors()
{
	// Array for fillin object redirectors
	TArray<UObjectRedirector*> RedirectorsToFixArray;

	//Load in asset registry module for later use
	IAssetRegistry& AssetRegistry =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	// Form a filter from the paths
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");

	//Use class path name for searching redirectors
	Filter.ClassPaths.Add(UObjectRedirector::StaticClass()->GetClassPathName());

	// Query for a list of assets in the selected paths
	TArray<FAssetData> AssetList;
	AssetRegistry.GetAssets(Filter, AssetList);

	if (AssetList.Num() == 0) return;

	TArray<FString> ObjectPaths;
	for (const FAssetData& Asset : AssetList)
	{
		ObjectPaths.Add(Asset.GetObjectPathString());
	}

	TArray<UObject*> Objects;
	const bool bAllowedToPromptToLoadAssets = true;
	const bool bLoadRedirects = true;

	if (AssetViewUtils::LoadAssetsIfNeeded(ObjectPaths, Objects, bAllowedToPromptToLoadAssets, bLoadRedirects))
	{
		// Transform Objects array to ObjectRedirectors array
		TArray<UObjectRedirector*> Redirectors;
		for (UObject* Object : Objects)
		{
			Redirectors.Add(CastChecked<UObjectRedirector>(Object));
		}

		// Load the asset tools module
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		AssetToolsModule.Get().FixupReferencers(Redirectors);
	}
}
#pragma endregion

#pragma region CustomEditorTab
void FSuperManagerModule::RegisterAdvancedDeletionTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AdvancedDeletion"), FOnSpawnTab::CreateRaw(this, &FSuperManagerModule::OnSpawnAdvancedDeletionTab)).SetDisplayName(FText::FromString(TEXT("Advanced Deletion")));
}

TSharedRef<SDockTab> FSuperManagerModule::OnSpawnAdvancedDeletionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return
		SNew(SDockTab).TabRole(ETabRole::NomadTab)
		[
			SNew(SAdvancedDeletionWidget)
			 .AssetsDataArray(GetAllAssetDataUnderSelectedFolder())
		];
}

TArray<TSharedPtr<FAssetData>> FSuperManagerModule::GetAllAssetDataUnderSelectedFolder()
{
	TArray<TSharedPtr<FAssetData>> AvailableAssetsData;

	TArray<FString> AssetsNames = UEditorAssetLibrary::ListAssets(SelectedFolderPath[0]);

	for (const FString& AssetName : AssetsNames)
	{
		if (AssetName.Contains(TEXT("Collections")) || AssetName.Contains(TEXT("Developers"))) { continue; }

		if (UEditorAssetLibrary::DoesAssetExist(AssetName) == false) { continue; }

		TArray<FString> AssetsReferences = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetName);

		//if (AssetsReferences.Num() == 0)
		{
			const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetName);

			AvailableAssetsData.Add(MakeShared<FAssetData>(Data));
		}
	}

	return AvailableAssetsData;
}

#pragma endregion

#pragma region ProcessDataForAdvancedDeletionWidget
bool FSuperManagerModule::DeleteSingleAsset(const FAssetData& AssetDataToDelete)
{
	TArray<FAssetData> AssetDataToDeleteArray;
	AssetDataToDeleteArray.Add(AssetDataToDelete);

	return ObjectTools::DeleteAssets(AssetDataToDeleteArray) > 0;
}

int32 FSuperManagerModule::DeleteMultipleAssets(const TArray<FAssetData>& AssetDataToDeleteArray)
{
	if (AssetDataToDeleteArray.Num() == 0)
	{
		return 0;
	}

	return ObjectTools::DeleteAssets(AssetDataToDeleteArray);
}

void FSuperManagerModule::ListUnusedAssets(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutUnusedAssetData)
{
	OutUnusedAssetData.Empty();

	for (const TSharedPtr<FAssetData>& DataPtr : AssetsDataToFilter)
	{
		FString AssetPath = DataPtr->GetSoftObjectPath().ToString();

		if (AssetPath.Contains(TEXT("Collections")) || AssetPath.Contains(TEXT("Developers"))) { continue; }

		if (UEditorAssetLibrary::DoesAssetExist(AssetPath) == false) { continue; }

		TArray<FString> AssetsReferences = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPath);

		if (AssetsReferences.Num() == 0)
		{
			OutUnusedAssetData.AddUnique(DataPtr);
		}
	}
}

void FSuperManagerModule::ListSameNameAssets(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutSameNameAssetData)
{
	OutSameNameAssetData.Empty();

	TMultiMap < FString, TSharedPtr<FAssetData>> AssetsInfoMultiMap;

	for (const TSharedPtr<FAssetData>& DataPtr : AssetsDataToFilter)
	{
		FString AssetPath = DataPtr->GetSoftObjectPath().ToString();

		if (AssetPath.Contains(TEXT("Collections")) || AssetPath.Contains(TEXT("Developers"))) { continue; }

		if (UEditorAssetLibrary::DoesAssetExist(AssetPath) == false) { continue; }

		AssetsInfoMultiMap.Emplace(DataPtr->AssetName.ToString(), DataPtr);
	}

	for (const TSharedPtr<FAssetData>& DataPtr : AssetsDataToFilter)
	{
		TArray<TSharedPtr<FAssetData>> DuplicatedAssetsData;
		AssetsInfoMultiMap.MultiFind(DataPtr->AssetName.ToString(), DuplicatedAssetsData);

		if (DuplicatedAssetsData.Num() <= 1) { continue; }

		for (const TSharedPtr<FAssetData>& DuplicatedAssetData : DuplicatedAssetsData)
		{
			if (DuplicatedAssetData->IsValid() == false) { continue; }

			OutSameNameAssetData.AddUnique(DuplicatedAssetData);
		}
	}

}

#pragma endregion

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)