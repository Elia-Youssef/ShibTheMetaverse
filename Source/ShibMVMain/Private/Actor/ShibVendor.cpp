#include "Actor/ShibVendor.h"


struct FShibItemDataTable;

AShibVendor::AShibVendor()
{
}

TArray<FShibItemDataTable> AShibVendor::UpdateVendorList()
{
	TArray<FShibItemDataTable> AvailableItems;
	for(auto VendorItem : VendorItems)
	{
		for(const auto RowNames : VendorItem.RowNames)
		{
			const FShibItemDataTable* NewItem = VendorItem.DataTable->FindRow<FShibItemDataTable>(RowNames, TEXT("Lookup Item"));

			AvailableItems.Add(*NewItem);
		}
	}

	return AvailableItems;
}
