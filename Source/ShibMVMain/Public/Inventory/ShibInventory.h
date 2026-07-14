// Copyright Shib LLC.
#pragma once

#include "Components/ActorComponent.h"
#include "ShibItemDataTable.h"
#include "UI/Navigation/ShibBasePage.h"
#include "ShibInventory.generated.h"

class UMetaverseApisSubsystem;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShibItemsUpdated);

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHIBMVMAIN_API UShibInventory : public UActorComponent
{
	GENERATED_BODY()

public:

#pragma region Construction & Initialization
	
	UShibInventory();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_InitializeInventory(int32 InUserId);

#pragma endregion Construction & Initialization

#pragma region ModifyInventory

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	UDataTable* FishTableRef;

	UPROPERTY(ReplicatedUsing=OnRep_ShibItems)
	TArray<FShibItem> ShibItems;
	
	UPROPERTY(BlueprintAssignable)
	FOnShibItemsUpdated OnShibItemsUpdated;
	
	/**Loot Luck increases and decreases depending on the rarity of items found.*/
	UPROPERTY()
	uint16 LootLuck = 0;
	
	/**Adds a new item to the inventory. Is compatible to have different quantities added in the new struct.*/
	UFUNCTION(BlueprintCallable)
	bool AddItemFromRowName(FName RowName, int32 Quantity);

	/**We should consider only using AddItemFromRowName.*/
	UFUNCTION(BlueprintCallable)
	bool AddItemFromStruct(FShibItem NewItem);

	/**Returns the amount of items that were removed.*/
	UFUNCTION(BlueprintCallable)
	int32 RemoveItemByRowName(FName RowName, int32 Quantity);

protected:
	
	UFUNCTION()
	void OnRep_ShibItems();

	/**This is only used for adding an item to a new slot if the item type does not already exist in the inventory.*/
	void AddUniqueItem(FShibItem& NewItem);

#pragma endregion ModifyInventory

#pragma region BuyingSelling

	/**This function will call BuyItemFromRowName using the RowName and Quantity as parameters.*/
	UFUNCTION(BlueprintCallable)
	bool BuyItemFromStruct(FShibItem NewItem);
	
	UFUNCTION(BlueprintCallable)
	bool BuyItemFromRowName(FName RowName, int32 Quantity);

	UFUNCTION(Server, Reliable)
	void Server_BuyItemFromRowName(FName RowName, int32 Quantity);

	UFUNCTION(BlueprintCallable)
	bool SellItemFromStruct(FShibItem SoldItem);
	
	UFUNCTION(BlueprintCallable)
	bool SellItemFromRowName(FName RowName, int32 Quantity);
	
	UFUNCTION(Server, Reliable)
	void Server_SellItemFromRowName(FName RowName, int32 Quantity);
	
	void ModifyCredit(float AmountToAdd);
	
#pragma endregion BuyingSelling
	
#pragma region HelperFunctions

public:

	UDataTable* GetDataTableByCategory(EShibItemCategory ItemCategory) const;
	
	UFUNCTION(BlueprintPure)
	TArray<FShibItem> GetItems();

	FShibItemDataTable* GetItemFromRowName(FName RowName) const;
	
	FName GetRowNameFromItem(const FShibItemDataTable& Item) const;
	
	int32 GetPlayerCurrency() const;
	
	int32 GetLootLuck() const;
	
	bool HasItem(FName RowName) const;

	int32 GetItemQuantity(FName RowName);

	bool HasEnoughCredit(FName RowName, int32 Quantity) const;

#pragma endregion HelperFunctions

#pragma region Luck
	
	void AdjustLootLuck(EShibItemTier ItemTier);
	
	void ChangeLootLuck(int32 Amount);
	
#pragma endregion Luck
	
#pragma region Database
	
private: // DATABASE
	FString LoadInventoryEndpoint = FString("/api/MV/GETMVINVENTORY");
	FString SaveInventoryEndpoint = FString("/api/MV/INSERTUPDATEINVENTORY");

	int32 UserId = -1;

	UPROPERTY(Replicated)
	FMvPlayerDetails Player;

	UPROPERTY()
	UMetaverseApisSubsystem* MvAPIs = nullptr;

	/**
	 * This variable stores the backend's data.
	 */
	UPROPERTY()
	TArray<FShibSavedItem> SavedItems;

	UFUNCTION(BlueprintCallable, Category = "Database")
	void LoadInventory();
	
	UFUNCTION(BlueprintCallable, Category = "Database")
	void SaveInventory();

	UFUNCTION(BlueprintCallable, Category = "Database")
	void LoadMvPlayer();
	
	UFUNCTION(BlueprintCallable, Category = "Database")
	void SaveMvPlayer();

#pragma endregion Database
};

