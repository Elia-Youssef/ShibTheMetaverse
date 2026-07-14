#include "..\..\Public\Inventory\ShibItemDataTable.h"

bool FShibItemDataTable::ItemIsValid()
{
	// Here we can check things like, ID is proper, Name, Quantity, ect.
	return true;
}

FShibItem FShibItemDataTable::CreateItem(FName RowID) const
{
	FShibItem NewItem;
	NewItem.ItemName = ItemName;
	NewItem.Price = Price;
	NewItem.Icon = Icon;
	NewItem.ItemCategory = ItemCategory;
	NewItem.ItemSubCategory = ItemSubCategory;
	NewItem.ItemTier = ItemTier;
	NewItem.RowName = RowID;

	return NewItem;
}