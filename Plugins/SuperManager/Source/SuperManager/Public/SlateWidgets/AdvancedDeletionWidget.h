// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SAdvancedDeletionWidget : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAdvancedDeletionWidget) {}
	
	SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>,AssetsDataArray)

	SLATE_ARGUMENT(FString,CurrentSelectedFolder)
	
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:

#pragma region ConstructionMethods
	TSharedRef<SListView <TSharedPtr<FAssetData>>> ConstructAssetsListView();

	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructComboBox();

	TSharedRef<SCheckBox> ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay);

	TSharedRef<STextBlock> ConstructTextBlock(const FString& TextContent, const FSlateFontInfo& Font, FColor Color = FColor::White, ETextJustify::Type Justify = ETextJustify::Left);

	TSharedRef<SButton> ConstructButton(const TSharedPtr<FAssetData>& AssetDataToDisplay);

	TSharedRef<SButton> ConstructDeleteAllButton();
	TSharedRef<SButton> ConstructSelectAllButton();
	TSharedRef<SButton> ConstructDeselectAllButton();
#pragma endregion

#pragma region EventsMethods
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable);
	void OnRowMouseButtonClick(TSharedPtr<FAssetData> ClickedData);

	TSharedRef<SWidget> OnGenerateComboBoxWidget(TSharedPtr<FString> SourceItem);
	void OnComboBoxSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo);

	void OnCheckStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData);
	
	FReply OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData);
	FReply OnDeleteAllButtonClicked();
	FReply OnSelectAllButtonClicked();
	FReply OnDeselectAllButtonClicked();
#pragma endregion

#pragma region HelperMethods
	FSlateFontInfo GetEmbossedTextFont(float Size = 10.0f);
	void RefreshAssetListView();
#pragma endregion

private:
	TArray<TSharedPtr<FAssetData>> AssetsDataUnderSelectedFolder;
	TArray<TSharedPtr<FAssetData>> DisplayedAssetsData;

	TSharedPtr<SListView <TSharedPtr<FAssetData>>> ConstructedAssetsListView;

	TArray<TSharedPtr<FAssetData>> SelectedAssetsToDelete;
	TArray<TSharedRef<SCheckBox>> CheckBoxesArray;

	TArray<TSharedPtr<FString>> ComboBoxSourceItems;
	TSharedPtr<STextBlock> ComboDisplayTextBlock;
};
