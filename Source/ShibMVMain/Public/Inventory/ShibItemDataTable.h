// Copyright Shib LLC.
#pragma once
#include "ShibItem.h"
#include "ShibItemDataTable.generated.h"

USTRUCT(BlueprintType)
struct FShibItemDataTable : public FTableRowBase
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "")
	FName ItemName = "Missing Item Name";

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "", meta = (ClampMin = "0.0"))
	float Price = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "")
	UTexture2D* Icon = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "")
	TEnumAsByte<EShibItemTier> ItemTier = IT_Common;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "")
	TEnumAsByte<EShibItemCategory> ItemCategory = IC_None;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "")
	TEnumAsByte<EShibItemSubCategory> ItemSubCategory = ISC_None;
	
	static bool ItemIsValid();

	/**Parse the RowName from the data table into the RowID*/
	FShibItem CreateItem(FName RowID) const;
};

