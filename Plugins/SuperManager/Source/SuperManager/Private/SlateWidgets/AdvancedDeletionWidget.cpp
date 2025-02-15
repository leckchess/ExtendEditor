// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/AdvancedDeletionWidget.h"
#include "SlateBasics.h"
#include "DebugHeader.h"
#include "SuperManager.h"
#include "../../../../../../../Plugins/Editor/EditorScriptingUtilities/Source/EditorScriptingUtilities/Public/EditorAssetLibrary.h"

#define ListALL TEXT("List All Available Assets")
#define ListUnused TEXT("List Unused Assets")
#define ListSameName TEXT("List Assets With Same Name")

void SAdvancedDeletionWidget::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

	AssetsDataUnderSelectedFolder.Empty();
	CheckBoxesArray.Empty();
	SelectedAssetsToDelete.Empty();

	ComboBoxSourceItems.Empty();
	ComboBoxSourceItems.Add(MakeShared<FString>(ListALL));
	ComboBoxSourceItems.Add(MakeShared<FString>(ListUnused));
	ComboBoxSourceItems.Add(MakeShared<FString>(ListSameName));

	AssetsDataUnderSelectedFolder = InArgs._AssetsDataArray;
	DisplayedAssetsData = AssetsDataUnderSelectedFolder;

	FSlateFontInfo TitleTextFont = GetEmbossedTextFont(30.f);

	ChildSlot
		[
			SNew(SVerticalBox)

				// Title Slot
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
						.Text(FText::FromString("Advanced Deletion"))
						.Font(TitleTextFont)
						.Justification(ETextJustify::Center)
						.ColorAndOpacity(FColor::White)
				]

				// dropdown to specify the listing condition
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							ConstructComboBox()
						]

						// help text
						+ SHorizontalBox::Slot()
						.FillWidth(0.6f)
						[
							ConstructTextBlock(TEXT("Specify the listing condition in DDL. Left mouse click to go to asset location"), GetEmbossedTextFont(8.0f), FColor::Green, ETextJustify::Center)
						]

						+ SHorizontalBox::Slot()
						.FillWidth(0.1f)
						[
							ConstructTextBlock(TEXT("Current Folder \n") + InArgs._CurrentSelectedFolder, GetEmbossedTextFont(8.0f), FColor::Green, ETextJustify::Center)
						]
				]

				// assets list
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Fill)
				[
					SNew(SScrollBox)

						+ SScrollBox::Slot()
						[
							ConstructAssetsListView()
						]

				]

				// buttons
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						[
							ConstructDeleteAllButton()
						]

						+ SHorizontalBox::Slot()
						[
							ConstructSelectAllButton()
						]

						+ SHorizontalBox::Slot()
						[
							ConstructDeselectAllButton()
						]
				]
		];
}

#pragma region ConstructionMethods
TSharedRef<SListView <TSharedPtr<FAssetData>>> SAdvancedDeletionWidget::ConstructAssetsListView()
{
	ConstructedAssetsListView = SNew(SListView <TSharedPtr<FAssetData>>)
		.ItemHeight(24.0f)
		.ListItemsSource(&DisplayedAssetsData)
		.OnGenerateRow(this, &SAdvancedDeletionWidget::OnGenerateRowForList)
		.OnMouseButtonClick(this, &SAdvancedDeletionWidget::OnRowMouseButtonClick);

	return ConstructedAssetsListView.ToSharedRef();
}

TSharedRef<SComboBox<TSharedPtr<FString>>> SAdvancedDeletionWidget::ConstructComboBox()
{
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructedComboBox = SNew(SComboBox < TSharedPtr<FString>>)
		.OptionsSource(&ComboBoxSourceItems)
		.OnGenerateWidget(this, &SAdvancedDeletionWidget::OnGenerateComboBoxWidget)
		.OnSelectionChanged(this, &SAdvancedDeletionWidget::OnComboBoxSelectionChanged)
		[
			SAssignNew(ComboDisplayTextBlock, STextBlock)
				.Text(FText::FromString(TEXT("List Assets Option")))
		];

	return ConstructedComboBox;
}

TSharedRef<SCheckBox> SAdvancedDeletionWidget::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SCheckBox> ConstructedCheckbox = SNew(SCheckBox)
		.Type(ESlateCheckBoxType::CheckBox)
		.OnCheckStateChanged(this, &SAdvancedDeletionWidget::OnCheckStateChanged, AssetDataToDisplay)
		.Visibility(EVisibility::Visible);


	CheckBoxesArray.AddUnique(ConstructedCheckbox);

	return ConstructedCheckbox;
}

TSharedRef<STextBlock> SAdvancedDeletionWidget::ConstructTextBlock(const FString& TextContent, const FSlateFontInfo& Font, FColor Color, ETextJustify::Type Justify)
{
	TSharedRef<STextBlock> ConstructedTextBlock = SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(Font)
		.ColorAndOpacity(Color)
		.Justification(Justify)
		.AutoWrapText(true);

	return ConstructedTextBlock;
}

TSharedRef<SButton> SAdvancedDeletionWidget::ConstructButton(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton = SNew(SButton)
		.Text(FText::FromString("Delete"))
		.OnClicked(this, &SAdvancedDeletionWidget::OnDeleteButtonClicked, AssetDataToDisplay);

	return ConstructedButton;
}

TSharedRef<SButton> SAdvancedDeletionWidget::ConstructDeleteAllButton()
{
	TSharedRef<SButton> ConstructedButton = SNew(SButton)
		.OnClicked(this, &SAdvancedDeletionWidget::OnDeleteAllButtonClicked);

	ConstructedButton->SetContent(ConstructTextBlock(TEXT("Delete All"), GetEmbossedTextFont(), FColor::White, ETextJustify::Center));

	return ConstructedButton;
}

TSharedRef<SButton> SAdvancedDeletionWidget::ConstructSelectAllButton()
{
	TSharedRef<SButton> ConstructedButton = SNew(SButton)
		.OnClicked(this, &SAdvancedDeletionWidget::OnSelectAllButtonClicked);

	ConstructedButton->SetContent(ConstructTextBlock(TEXT("Select All"), GetEmbossedTextFont(), FColor::White, ETextJustify::Center));

	return ConstructedButton;
}

TSharedRef<SButton> SAdvancedDeletionWidget::ConstructDeselectAllButton()
{
	TSharedRef<SButton> ConstructedButton = SNew(SButton)
		.OnClicked(this, &SAdvancedDeletionWidget::OnDeselectAllButtonClicked);

	ConstructedButton->SetContent(ConstructTextBlock(TEXT("Deselect All"), GetEmbossedTextFont(), FColor::White, ETextJustify::Center));

	return ConstructedButton;
}

#pragma endregion

#pragma region EventsMethods
TSharedRef<ITableRow> SAdvancedDeletionWidget::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable)
{
	if (AssetDataToDisplay->IsValid() == false)
	{
		return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable);
	}

	const FString DisplayAssetClassName = AssetDataToDisplay->GetClass()->GetName();
	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();

	FSlateFontInfo EmbossedTextFont = GetEmbossedTextFont();

	TSharedRef<STableRow <TSharedPtr<FAssetData>>> ListViewRowWidget =
		SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable).Padding(FMargin(5.0f))
		[
			SNew(SHorizontalBox)

				// check box
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.FillWidth(0.05f)
				[
					ConstructCheckBox(AssetDataToDisplay)
				]

				//Display asset class name
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Fill)
				.FillWidth(0.2f)
				[
					ConstructTextBlock(DisplayAssetClassName, EmbossedTextFont, FColor::Emerald)
				]

				// actual asset name
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Fill)
				.FillWidth(0.2f)
				[
					ConstructTextBlock(DisplayAssetName, EmbossedTextFont)
				]

				// button
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Fill)
				[
					ConstructButton(AssetDataToDisplay)
				]
		];

	return ListViewRowWidget;
}

void SAdvancedDeletionWidget::OnRowMouseButtonClick(TSharedPtr<FAssetData> ClickedData)
{
	FSuperManagerModule& SuperManagerModule = FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	SuperManagerModule.SyncCBToClickedAsset(ClickedData->GetSoftObjectPath().ToString());

	DebugHeader::Print(ClickedData->AssetName.ToString(), FColor::Cyan);
}

TSharedRef<SWidget> SAdvancedDeletionWidget::OnGenerateComboBoxWidget(TSharedPtr<FString> SourceItem)
{
	TSharedRef<STextBlock> ConstructedComboText =
		SNew(STextBlock).Text(FText::FromString(*SourceItem.Get()));

	return ConstructedComboText;
}

void SAdvancedDeletionWidget::OnComboBoxSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo)
{
	ComboDisplayTextBlock->SetText(FText::FromString(*SelectedOption.Get()));

	FSuperManagerModule& SuperManagerModule = FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));

	// pass data to our module to filter
	if (*SelectedOption.Get() == ListALL)
	{
		DisplayedAssetsData = AssetsDataUnderSelectedFolder;
	}
	else if (*SelectedOption.Get() == ListUnused)
	{
		SuperManagerModule.ListUnusedAssets(AssetsDataUnderSelectedFolder, DisplayedAssetsData);
	}
	else if (*SelectedOption.Get() == ListSameName)
	{
		SuperManagerModule.ListSameNameAssets(AssetsDataUnderSelectedFolder, DisplayedAssetsData);
	}
	else
	{
		return;
	}

	RefreshAssetListView();
}

void SAdvancedDeletionWidget::OnCheckStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		if (SelectedAssetsToDelete.Contains(AssetData))
		{
			SelectedAssetsToDelete.Remove(AssetData);
		}
		break;

	case ECheckBoxState::Checked:
		SelectedAssetsToDelete.AddUnique(AssetData);
		break;

	case ECheckBoxState::Undetermined:
		break;

	default:
		break;
	}
}

FReply SAdvancedDeletionWidget::OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	if (ClickedAssetData->IsValid() == false) { return FReply::Handled(); }

	FSuperManagerModule& SuperManagerModule = FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	if (SuperManagerModule.DeleteSingleAsset(*ClickedAssetData.Get()))
	{
		if (AssetsDataUnderSelectedFolder.Contains(ClickedAssetData))
		{
			AssetsDataUnderSelectedFolder.Remove(ClickedAssetData);
		}

		if (DisplayedAssetsData.Contains(ClickedAssetData))
		{
			DisplayedAssetsData.Remove(ClickedAssetData);
		}

		// Refresh The List
		RefreshAssetListView();
	}

	return FReply::Handled();
}

FReply SAdvancedDeletionWidget::OnDeleteAllButtonClicked()
{
	if (SelectedAssetsToDelete.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No assets currently selected"));
		return FReply::Handled();
	}

	TArray<FAssetData> AssetsDataToDelete;

	for (TSharedPtr<FAssetData>& AssetsDataPtr : SelectedAssetsToDelete)
	{
		AssetsDataToDelete.Add(*AssetsDataPtr.Get());
	}

	FSuperManagerModule& SuperManagerModule = FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	if (int32 DeletedAssets = SuperManagerModule.DeleteMultipleAssets(AssetsDataToDelete))
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, FString::FromInt(DeletedAssets) + TEXT(" Asstes Deleted Successfully"));

		for (TSharedPtr<FAssetData>& AssetsDataPtr : SelectedAssetsToDelete)
		{
			if (UEditorAssetLibrary::DoesAssetExist(AssetsDataPtr.Get()->AssetName.ToString()) == true) { continue; }

			if (AssetsDataUnderSelectedFolder.Contains(AssetsDataPtr))
			{
				AssetsDataUnderSelectedFolder.Remove(AssetsDataPtr);
			}

			if (DisplayedAssetsData.Contains(AssetsDataPtr))
			{
				DisplayedAssetsData.Remove(AssetsDataPtr);
			}
		}

		// Refresh The List
		RefreshAssetListView();
	}

	return FReply::Handled();
}

FReply SAdvancedDeletionWidget::OnSelectAllButtonClicked()
{
	if (CheckBoxesArray.Num() == 0)
	{
		return FReply::Handled();
	}

	for (TSharedRef<SCheckBox>& CheckBox : CheckBoxesArray)
	{
		if (CheckBox->IsChecked()) { continue; }

		CheckBox->ToggleCheckedState();
	}

	return FReply::Handled();
}

FReply SAdvancedDeletionWidget::OnDeselectAllButtonClicked()
{
	if (CheckBoxesArray.Num() == 0)
	{
		return FReply::Handled();
	}

	for (TSharedRef<SCheckBox>& CheckBox : CheckBoxesArray)
	{
		if (CheckBox->IsChecked() == false) { continue; }

		CheckBox->ToggleCheckedState();
	}
	return FReply::Handled();
}
#pragma endregion

#pragma region HelperMethods
FSlateFontInfo SAdvancedDeletionWidget::GetEmbossedTextFont(float Size)
{
	FSlateFontInfo TextFontInfo = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	TextFontInfo.Size = Size;

	return TextFontInfo;
}

void SAdvancedDeletionWidget::RefreshAssetListView()
{
	SelectedAssetsToDelete.Empty();
	CheckBoxesArray.Empty();

	if (ConstructedAssetsListView.IsValid())
	{
		ConstructedAssetsListView->RebuildList();
	}
}
#pragma endregion
