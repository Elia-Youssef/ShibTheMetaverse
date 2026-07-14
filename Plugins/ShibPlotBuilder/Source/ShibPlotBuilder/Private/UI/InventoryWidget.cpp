// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InventoryWidget.h"

#include "Components/BuildingComponent.h"
#include "Tools/BuildTypes.h"
#include "Engine/DataTable.h"
#include "Player/BuildController.h"

void UInventoryWidget::InitializeInventory(ABuildController* InController, UDataTable* InObjectsTable)
{
	Controller = InController;
	ObjectsTable = InObjectsTable;
}

void UInventoryWidget::SetWidgetMode(const EWidgetMode InWidgetMode, bool bClickedAgain) {
	WidgetMode = InWidgetMode;
	ItemsDetails.Empty();
	
	if (bClickedAgain)
	{
		UpdateItemsWidgets();
		
		Controller->BuildingComponent->SetSelectedActorToSpawn(FName("None"));
		Controller->BuildingComponent->OnInteractionModeChange(EInteractionMode::None);
		return;
	}

	if (InWidgetMode == EWidgetMode::Build)
	{
		LoadBuildItems();
	} else
	{
		UpdateItemsWidgets();
		
		Controller->BuildingComponent->SetSelectedActorToSpawn(FName("None"));
		Controller->BuildingComponent->OnInteractionModeChange(EInteractionMode::None);
	}
}

FItemArrowsAvailability UInventoryWidget::GetAvailableArrows()
{
	const TArray<UUserWidget*> WidgetsToCheck = bIsMenuExpanded ? ExpandedMenuItems : MenuItems;
	const int32 MaxColumns = bIsMenuExpanded ? ExpandedMenuMaxColumns : MenuMaxColumns;
	const int32 SelectedItemWidgetIndex = WidgetsToCheck.Find(SelectedItemWidget);

	const int32 CurrentColumn = SelectedItemWidgetIndex % MaxColumns;
	const int32 CurrentRow = SelectedItemWidgetIndex / MaxColumns;

	FItemArrowsAvailability ItemArrowsAvailability;
	ItemArrowsAvailability.bUp = CurrentRow > 0;
	ItemArrowsAvailability.bDown = (SelectedItemWidgetIndex + MaxColumns) < WidgetsToCheck.Num();
	ItemArrowsAvailability.bLeft = CurrentColumn > 0;
	ItemArrowsAvailability.bRight = CurrentColumn < MaxColumns - 1 && (SelectedItemWidgetIndex + 1) < WidgetsToCheck.Num();

	return ItemArrowsAvailability;
}

void UInventoryWidget::OnSelect(const FVector& InputVector)
{
	FItemArrowsAvailability ItemArrowsAvailability = GetAvailableArrows();
	
	const TArray<UUserWidget*> WidgetsToCheck = bIsMenuExpanded ? ExpandedMenuItems : MenuItems;
	const int32 MaxColumns = bIsMenuExpanded ? ExpandedMenuMaxColumns : MenuMaxColumns;
	const int32 SelectedItemWidgetIndex = WidgetsToCheck.Find(SelectedItemWidget);

	int32 NewIndex = 0;
	
	if (InputVector.X < 0 && ItemArrowsAvailability.bLeft)
	{
		NewIndex = SelectedItemWidgetIndex - 1;
	}
	else if (InputVector.X > 0 && ItemArrowsAvailability.bRight)
	{
		NewIndex = SelectedItemWidgetIndex + 1;
	}
	else if (InputVector.Y < 0 && ItemArrowsAvailability.bDown)
	{
		NewIndex = SelectedItemWidgetIndex + MaxColumns;
	}
	else if (InputVector.Y > 0 && ItemArrowsAvailability.bUp)
	{
		NewIndex = SelectedItemWidgetIndex - MaxColumns;
	}
	else return;
	
	SelectedItemWidget = WidgetsToCheck[NewIndex];
	SelectedItem = ItemsDetails[NewIndex];
	Controller->BuildingComponent->SetSelectedActorToSpawn(SelectedItem.Tag);
	
	ItemArrowsAvailability = GetAvailableArrows();
	UpdateHighlight(ItemArrowsAvailability, GetNewHighlightLocation());
}

void UInventoryWidget::OnHover(UUserWidget* Widget)
{
	SelectedItemWidget = Widget;
	
	const TArray<UUserWidget*> WidgetsToCheck = bIsMenuExpanded ? ExpandedMenuItems : MenuItems;
	const int32 SelectedItemWidgetIndex = WidgetsToCheck.Find(SelectedItemWidget);
	
	SelectedItem = ItemsDetails[SelectedItemWidgetIndex];
	Controller->BuildingComponent->SetSelectedActorToSpawn(SelectedItem.Tag);
	
	const FItemArrowsAvailability ItemArrowsAvailability = GetAvailableArrows();
	UpdateHighlight(ItemArrowsAvailability, GetNewHighlightLocation());
}

int32 UInventoryWidget::GetNumberOfRows(const bool bExpanded) const
{
	const int32 MaxColumns = bExpanded ? ExpandedMenuMaxColumns : MenuMaxColumns;
	const int32 NumFullRows = ItemsDetails.Num() / MaxColumns;
	const int32 NumAdditionalRows = ItemsDetails.Num() % MaxColumns > 0 ? 1 : 0;

	return NumFullRows + NumAdditionalRows;
}

TArray<FWidgetItem> UInventoryWidget::GetItemsInRow(const int32 RowIndex, const bool bExpanded)
{
	const int32 MaxColumns = bExpanded ? ExpandedMenuMaxColumns : MenuMaxColumns;
	TArray<FWidgetItem> Slice;

	const int32 Start = RowIndex * MaxColumns;
	int32 End = Start + MaxColumns - 1;

	if (!ItemsDetails.IsValidIndex(Start)) return Slice;
	if (!ItemsDetails.IsValidIndex(End)) End = ItemsDetails.Num() - 1;
	
	for (int32 i = Start; i <= End; i++)
	{
		Slice.Add(ItemsDetails[i]);
	}
	
	return Slice;
}

FVector2D UInventoryWidget::GetNewHighlightLocation() const
{
	FVector2D Location;
	
	const TArray<UUserWidget*> WidgetsToCheck = bIsMenuExpanded ? ExpandedMenuItems : MenuItems;
	const int32 MaxColumns = bIsMenuExpanded ? ExpandedMenuMaxColumns : MenuMaxColumns;
	const int32 SelectedItemWidgetIndex = WidgetsToCheck.Find(SelectedItemWidget);

	Location.X = SelectedItemWidgetIndex % MaxColumns * (2 * ItemWidgetPadding.X + ItemWidgetSize.X) + ItemWidgetPadding.X;
	Location.Y = SelectedItemWidgetIndex / MaxColumns * (2 * ItemWidgetPadding.Y + ItemWidgetSize.Y) + ItemWidgetPadding.Y;

	return Location;
}

void UInventoryWidget::LoadBuildItems()
{
	const FName Parent = ParentsTree.IsEmpty() ? FName("None") : ParentsTree.Last();
	TArray<FName> RowNames = ObjectsTable->GetRowNames();

	TArray<FWidgetItem> TempItems;
	for (const FName RowName : RowNames) {
		const FObjectDetails* Row = ObjectsTable->FindRow<FObjectDetails>(RowName, "");

		if (Row->ObjectType.ParentTag == Parent || Row->ObjectType.Tag == Parent)
		{
			FWidgetItem WidgetItem;
			WidgetItem.Tag = RowName;
			WidgetItem.Title = Row->Title.ToString();
			WidgetItem.PreviewImage = Row->PreviewImage;
				
			TempItems.Add(WidgetItem);
		}
	}

	if (TempItems.IsEmpty()) return;

	ItemsDetails = TempItems;
	
	UpdateItemsWidgets();
	
	SelectedItem = ItemsDetails[0];
	SelectedItemWidget = MenuItems[0];
	Controller->BuildingComponent->SetSelectedActorToSpawn(SelectedItem.Tag);
	Controller->BuildingComponent->OnInteractionModeChange(EInteractionMode::Build);
}

void UInventoryWidget::OnEnter()
{
	if (!Controller->BuildingComponent->SelectedActorToSpawn.bHasChildren) return;
	if (!ParentsTree.IsEmpty() && ParentsTree.Last() == Controller->BuildingComponent->SelectedActorToSpawn.ObjectType.Tag) return;
	
	ItemsDetails.Empty();
	ParentsTree.Add(Controller->BuildingComponent->SelectedActorToSpawn.ObjectType.Tag);
	LoadBuildItems();
}

void UInventoryWidget::OnBack()
{
	if (ParentsTree.IsEmpty()) return;
	
	ItemsDetails.Empty();
	ParentsTree.Pop();
	LoadBuildItems();
}
