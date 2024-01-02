
#include "AssetActions\QuickAssetAction.h"
#include "DebugHeader.h"

#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"

#include "AssetToolsModule.h"
#include "AssetViewUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"

void UQuickAssetAction::DuplicateAssets(int32 NumOfDuplicates)
{
	if (NumOfDuplicates <= 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter a VALID number"));
		return;
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 Counter = 0;

	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		for (int32 i = 0; i < NumOfDuplicates; i++)
		{
			const FString SourceAssetPath = SelectedAssetData.GetSoftObjectPath().ToString();
			const FString NewDuplicatedAssetName = SelectedAssetData.AssetName.ToString() + TEXT("_") + FString::FromInt(i + 1);
			const FString NewPathName = FPaths::Combine(SelectedAssetData.PackagePath.ToString(), NewDuplicatedAssetName);

			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewPathName))
			{
				UEditorAssetLibrary::SaveAsset(NewPathName, false);
				++Counter;
			}
		}
	}

	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully duplicated " + FString::FromInt(Counter) + " assets"));
	}
}

void UQuickAssetAction::AddPrefixes()
{
	TArray<UObject*> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;

	for (UObject* SelectedAsset : SelectedAssets)
	{
		if (SelectedAsset == nullptr) { continue; }

		FString* PrefixFound = PrefixMap.Find(SelectedAsset->GetClass());
		if (PrefixFound == nullptr || PrefixFound->IsEmpty())
		{
			DebugHeader::Print(TEXT("Failed to find prefix for calss ") + SelectedAsset->GetClass()->GetName(), FColor::Red);
			continue;
		}

		FString OldName = SelectedAsset->GetName();
		if (OldName.StartsWith(*PrefixFound))
		{
			DebugHeader::Print(OldName + TEXT(" already has prefix added "), FColor::Red);
			continue;
		}

		if (SelectedAsset->IsA<UMaterialInstanceConstant>())
		{
			OldName.RemoveFromStart("M_");
			OldName.RemoveFromEnd("_inst");

			if (OldName.Find("_inst"))
			{
				int32 StartRemovingIndex = -1;
				if (OldName.FindLastChar('_', StartRemovingIndex))
				{
					OldName.RemoveAt(StartRemovingIndex + 1, 4);
				}
			}
		}

		const FString NewNameWithPrefix = *PrefixFound + OldName;

		UEditorUtilityLibrary::RenameAsset(SelectedAsset, NewNameWithPrefix);

		++Counter;
	}

	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully renamed " + FString::FromInt(Counter) + " assets"));
	}
}

void UQuickAssetAction::AddPrefixes_Batched()
{
	for (TPair<UClass*, FString> MappedClass : PrefixMap)
	{
		TArray<UObject*> AssetsOfClass = UEditorUtilityLibrary::GetSelectedAssetsOfClass(MappedClass.Key);

		if (AssetsOfClass.Num() == 0) { continue; }

		FString Prefix = MappedClass.Value;
		TArray<FAssetRenameData> AssetsAndNames;

		for (UObject* AssetOfClass : AssetsOfClass)
		{
			FString OldName = AssetOfClass->GetName();
			if (OldName.StartsWith(*Prefix))
			{
				DebugHeader::Print(OldName + TEXT(" already has prefix added "), FColor::Red);
				continue;
			}

			if (AssetOfClass->IsA<UMaterialInstanceConstant>())
			{
				OldName.RemoveFromStart("M_");
				OldName.RemoveFromEnd("_inst");

				if (OldName.Find("_inst"))
				{
					int32 StartRemovingIndex = -1;
					if (OldName.FindLastChar('_', StartRemovingIndex))
					{
						OldName.RemoveAt(StartRemovingIndex + 1, 4);
					}
				}
			}

			const FString PackagePath = FPackageName::GetLongPackagePath(AssetOfClass->GetOutermost()->GetName());
			const FString NewNameWithPrefix = *Prefix + OldName;

			AssetsAndNames.Add(FAssetRenameData(AssetOfClass, PackagePath, NewNameWithPrefix));
		}

		if (AssetsAndNames.Num() == 0) { continue; }

		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
		EAssetRenameResult RenamingAsetsResult = AssetToolsModule.Get().RenameAssetsWithDialog(AssetsAndNames);

		if (RenamingAsetsResult == EAssetRenameResult::Success)
		{
			DebugHeader::ShowNotifyInfo(TEXT("Successfully renamed ") + FString::FromInt(AssetsOfClass.Num()) + TEXT(" assets of class ") + MappedClass.Key->GetClass()->GetName());
		}
	}
}

void UQuickAssetAction::RemoveUnusedAssets()
{
	FixupRedirectors();

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> UnusedAssetsData;

	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		TArray<FString> AssetRefrencers = UEditorAssetLibrary::FindPackageReferencersForAsset(SelectedAssetData.GetSoftObjectPath().ToString());

		if (AssetRefrencers.Num() == 0)
		{
			UnusedAssetsData.Add(SelectedAssetData);
		}
	}

	if (UnusedAssetsData.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused assets found amoung selected assets"), false);
		return;
	}

	int32 NumOfAssetsDeleted = ObjectTools::DeleteAssets(UnusedAssetsData);

	if (NumOfAssetsDeleted == 0) { return; }

	DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted " + FString::FromInt(NumOfAssetsDeleted) + " unused assets"));

}

void UQuickAssetAction::FixupRedirectors()
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
