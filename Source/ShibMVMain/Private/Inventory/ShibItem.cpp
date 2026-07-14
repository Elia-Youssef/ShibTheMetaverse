// Copyright Shiba Inu Games LLC.
#include "Inventory/ShibItem.h"

bool FShibItem::ItemIsValid() const
{
	// Here we can check things like, ID is proper, Name, Quantity, ect.
	return Quantity > 0 && ! ItemName.IsNone() && ! RowName.IsNone();
}

float FShibItem::GetTotalPrice() const
{
	return FMath::Max(Quantity * Price, 0.0f);
}
