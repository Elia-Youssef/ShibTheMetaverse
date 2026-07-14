// Copyright Shib LLC.
#pragma once

#include "CoreMinimal.h"
#include "ShibItem.generated.h"

UENUM(BlueprintType)
enum EShibItemTier
{
	IT_Common   UMETA(DisplayName = "Common"),
	IT_Uncommon UMETA(DisplayName = "Uncommon"),
	IT_Rare     UMETA(DisplayName = "Rare"),
	IT_Epic     UMETA(DisplayName = "Epic"),
};

UENUM(BlueprintType)
enum EShibItemCategory
{
	IC_None     UMETA(DisplayName = "None"),
	IC_Fishing  UMETA(DisplayName = "Fishing"),
};

UENUM(BlueprintType)
enum EShibItemSubCategory
{
	ISC_None				UMETA(DisplayName = "None"),
	ISC_RawFish				UMETA(DisplayName = "RawFish"),
	ISC_FishingSupplies		UMETA(DisplayName = "FishingSupplies"),
};

USTRUCT(BlueprintType)
struct FShibItem
{
	GENERATED_BODY()

public:
	/**RowName is used as a 'Unique ID'. Unlike ItemName which is flexible and can overlap with other items.*/
	UPROPERTY(BlueprintReadWrite, Category = "")
	FName RowName = NAME_None;
	
	UPROPERTY(BlueprintReadWrite, Category = "")
	FName ItemName = NAME_None;

	UPROPERTY(BlueprintReadWrite, Category = "")
	float Price = 0;

	UPROPERTY(BlueprintReadWrite, Category = "")
	UTexture2D* Icon = nullptr;
	
	UPROPERTY(BlueprintReadWrite, Category = "")
	TEnumAsByte<EShibItemCategory> ItemCategory = IC_None;

	UPROPERTY(BlueprintReadWrite, Category = "")
	TEnumAsByte<EShibItemSubCategory> ItemSubCategory = ISC_None;

	UPROPERTY(BlueprintReadWrite, Category = "")
	TEnumAsByte<EShibItemTier> ItemTier = IT_Common;
	
	UPROPERTY(BlueprintReadWrite, Category = "")
	int32 Quantity = 1;

	UPROPERTY(BlueprintReadWrite, Category = "")
	int32 IndexSlot = 0;
	
	bool ItemIsValid() const;

	float GetTotalPrice() const;
};
