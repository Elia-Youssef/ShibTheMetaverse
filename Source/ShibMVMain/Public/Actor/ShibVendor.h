#pragma once
#include "Inventory/ShibItemDataTable.h"
#include "ShibVendor.generated.h"

USTRUCT(BlueprintType)
struct FShibVendorItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vendor")
	UDataTable* DataTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vendor")
	TArray<FName> RowNames;
};

UCLASS(BlueprintType)
class SHIBMVMAIN_API AShibVendor : public AActor
{
	GENERATED_BODY()
public:
	AShibVendor();
	
	UPROPERTY(EditAnywhere, Category = "Vendor")
	TArray<FShibVendorItem> VendorItems;

	UFUNCTION(BlueprintCallable)
	TArray<FShibItemDataTable> UpdateVendorList();
	
};
