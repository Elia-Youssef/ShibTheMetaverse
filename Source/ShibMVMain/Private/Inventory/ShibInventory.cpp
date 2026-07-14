#include "Inventory/ShibInventory.h"

#include "Kismet/GameplayStatics.h"
#include "Metaverse/MetaverseApisSubsystem.h"
#include "Net/UnrealNetwork.h"

#pragma region Construction & Initialization

UShibInventory::UShibInventory()
{
}

void UShibInventory::Server_InitializeInventory_Implementation(int32 InUserId)
{
	UserId = InUserId;
	if (UserId <= 0)
	{
		return;
	}

	auto* ShibGI = UGameplayStatics::GetGameInstance(this);
	if (!ShibGI) return;

	MvAPIs = ShibGI->GetSubsystem<UMetaverseApisSubsystem>();
	if (!MvAPIs) return;

	LoadInventory();
	LoadMvPlayer();
}

#pragma endregion Construction & Initialization

#pragma region ModifyInventory

bool UShibInventory::AddItemFromRowName(FName RowName, int32 Quantity)
{
	// Retrieve the item data from the row name
	const FShibItemDataTable* ItemFromDataTable = GetItemFromRowName(RowName);
	if ( ! ItemFromDataTable)
	{
		return false;
	}

	// Check if the item already exists in the inventory
	for (FShibItem& ShibItem : ShibItems)
	{
		// Check if item name, size and tier match
		if (ShibItem.RowName == RowName)
			{
				// Update the quantity
				ShibItem.Quantity += Quantity;
				SaveInventory();
				return true;
			}
	}

	// Create a new item if exact item doesn't exist in the inventory
	FShibItem NewInventoryItem = ItemFromDataTable->CreateItem(RowName);
	NewInventoryItem.Quantity = Quantity;
	
	// Add the new item with a unique slot
	AddUniqueItem(NewInventoryItem);
	SaveInventory();

	return true;
}

bool UShibInventory::AddItemFromStruct(FShibItem NewItem)
{
	return AddItemFromRowName(NewItem.RowName, NewItem.Quantity);
}

void UShibInventory::AddUniqueItem(FShibItem& NewItem)
{
	if (NewItem.ItemIsValid())
	{
		// Find the lowest available slot index
		TSet<int32> UsedSlots;
		for (const FShibItem& Item : ShibItems)
		{
			UsedSlots.Add(Item.IndexSlot);
		}

		// Determine the lowest unused slot index
		int32 NewSlotIndex = 0;
		while (UsedSlots.Contains(NewSlotIndex))
		{
			NewSlotIndex++;
		}
		
		NewItem.IndexSlot = NewSlotIndex;
		ShibItems.Add(NewItem);
	}
}

int32 UShibInventory::RemoveItemByRowName(FName RowName, int32 Quantity)
{
	if (ShibItems.IsEmpty() || Quantity <= 0)
	{
		return 0;
	}

	for (int32 i = 0; i < ShibItems.Num(); i++)
	{
		FShibItem& ShibItem = ShibItems[i]; // Reference to modify directly
		
		if (ShibItem.RowName == RowName)
		{
			const int32 RemovedAmount = FMath::Min(ShibItem.Quantity, Quantity);

			if (ShibItem.Quantity > RemovedAmount)
			{
				// Reduce the quantity and return the amount removed
				ShibItem.Quantity -= RemovedAmount;
			}
			else
			{
				// Remove the item if quantity reaches 0
				ShibItems.RemoveAt(i);
			}

			return RemovedAmount;
		}
	}

	return 0; // Item was not found
}


void UShibInventory::OnRep_ShibItems()
{
	OnShibItemsUpdated.Broadcast();
}

#pragma endregion ModifyInventory

#pragma region BuyingSelling

bool UShibInventory::BuyItemFromStruct(FShibItem NewItem)
{
	if(NewItem.ItemIsValid())
	{
		return BuyItemFromRowName(NewItem.RowName, NewItem.Quantity);
	}
	
	return false;
}

bool UShibInventory::BuyItemFromRowName(FName RowName, int32 Quantity)
{
	if (const FShibItemDataTable* ItemFromDataTable = GetItemFromRowName(RowName))
	{
		if( ! HasEnoughCredit(RowName, Quantity))
		{
			// Player does not have enough credit to buy this quantity of the item
			return false;
		}
		
		// If we are client, make sure only server processes this
		if(const AActor* ActorOwner = GetOwner())
		{
			if( ! ActorOwner->HasAuthority())
			{
				Server_BuyItemFromRowName(RowName, Quantity);
				return false;
			}
		}

		// Only server calls this
		// Creates an item struct to get the total price based on quantity
		FShibItem NewItem = ItemFromDataTable->CreateItem(RowName);
		NewItem.Quantity = Quantity;
		if(AddItemFromStruct(NewItem)) // Adds item to inventory
		{
			// Subtracts credit depending on quantity
			ModifyCredit(-NewItem.GetTotalPrice());
		}
	}

	return false;
}

void UShibInventory::Server_BuyItemFromRowName_Implementation(FName RowName, int32 Quantity)
{
	BuyItemFromRowName(RowName, Quantity);
}

bool UShibInventory::SellItemFromStruct(FShibItem SoldItem)
{
	// Check if inventory has enough quantity of requested sell
	if(SoldItem.ItemIsValid() && GetItemQuantity(SoldItem.RowName) >= SoldItem.Quantity)
	{
		return SellItemFromRowName(SoldItem.RowName, SoldItem.Quantity);
	}
	
	return false;
}

bool UShibInventory::SellItemFromRowName(FName RowName, int32 Quantity)
{
	if (const FShibItemDataTable* ItemFromDataTable = GetItemFromRowName(RowName))
	{
		// If we are client, make sure only server processes this
		if(const AActor* ActorOwner = GetOwner())
		{
			if( ! ActorOwner->HasAuthority())
			{
				Server_SellItemFromRowName(RowName, Quantity);
				return false;
			}
		}

		// Only server calls this
		if(const int32 RemovedQuantity = RemoveItemByRowName(RowName, Quantity))
		{
			// Creates a copy of the item to get Total price based on remaining quantity
			FShibItem NewItem = ItemFromDataTable->CreateItem(RowName);
			NewItem.Quantity = RemovedQuantity;
			
			// Adds credit depending on how many items were sold
			ModifyCredit(NewItem.GetTotalPrice());
		}
	}

	return false;
}

void UShibInventory::Server_SellItemFromRowName_Implementation(FName RowName, int32 Quantity)
{
	SellItemFromRowName(RowName, Quantity);
}

void UShibInventory::ModifyCredit(float AmountToAdd)
{
	Player.ShibCredit += AmountToAdd;
	UE_LOG(LogTemp, Warning, TEXT("Player Credit: %d"), Player.ShibCredit);
}

#pragma endregion BuyingSelling

#pragma region HelperFunctions

FShibItemDataTable* UShibInventory::GetItemFromRowName(FName RowName) const
{
	if ( ! FishTableRef) { return nullptr; } // Ensure data table is valid
	if (RowName.IsNone()) { return nullptr; }
	
	FShibItemDataTable* NewItem = FishTableRef->FindRow<FShibItemDataTable>(RowName, TEXT("Lookup Item"));

	if ( ! NewItem || ! NewItem->ItemIsValid()) { return nullptr; }
	
	return NewItem;
}

FName UShibInventory::GetRowNameFromItem(const FShibItemDataTable& Item) const
{
	if ( ! FishTableRef) { return NAME_None; } // Ensure DataTableRef is valid
	
	// Get the row map from the DataTable
	for (const TPair<FName, uint8*>& Row : FishTableRef->GetRowMap())
	{
		// Cast the row data to FShibItemDataTable
		FShibItemDataTable* DataTableItem = reinterpret_cast<FShibItemDataTable*>(Row.Value);
		if (DataTableItem && DataTableItem->ItemName == Item.ItemName)
		{
			return Row.Key; // Return the row name if item names match
		}
	}

	return NAME_None; // Return None if no matching item is found
}

int32 UShibInventory::GetPlayerCurrency() const
{
	return Player.ShibCredit;
}

int32 UShibInventory::GetLootLuck() const
{
	return LootLuck;
}

bool UShibInventory::HasItem(FName RowName) const
{
	for(const auto &ShibItem : ShibItems)
	{
		if(ShibItem.RowName == RowName)
		{
			return true;
		}
	}

	return false;
}

int32 UShibInventory::GetItemQuantity(FName RowName)
{
	for(const auto &ShibItem : ShibItems)
	{
		if(ShibItem.RowName == RowName)
		{
			return ShibItem.Quantity;
		}
	}

	return 0;
}

bool UShibInventory::HasEnoughCredit(FName RowName, int32 Quantity) const
{
	if (const FShibItemDataTable* ItemFromDataTable = GetItemFromRowName(RowName))
	{
		const float TotalPrice = ItemFromDataTable->Price * Quantity;
		return GetPlayerCurrency() >= TotalPrice;
	}

	return false;
}

UDataTable* UShibInventory::GetDataTableByCategory(EShibItemCategory ItemCategory) const
{
	switch (ItemCategory)
	{
		case IC_Fishing: return FishTableRef;
		default: return nullptr;
	}
}

TArray<FShibItem> UShibInventory::GetItems()
{
	return ShibItems;
}

#pragma endregion HelperFunctions

#pragma region Luck

void UShibInventory::AdjustLootLuck(EShibItemTier ItemTier)
{
	// TODO expose variables
	constexpr int32 CommonLuckChange = 2;
	constexpr int32 UncommonLuckChange = 0;
	constexpr int32 RareLuckChange = -3;
	constexpr int32 EpicLuckChange = -5;

	int32 LuckChange = 0;

	switch (ItemTier)
	{
	case IT_Common:
		LuckChange = CommonLuckChange;
		break;
	case IT_Uncommon:
		LuckChange = UncommonLuckChange;
		break;
	case IT_Rare:
		LuckChange = RareLuckChange;
		break;
	case IT_Epic:
		LuckChange = EpicLuckChange;
		break;
	default:
		break;
	}

	ChangeLootLuck(LuckChange);
}

void UShibInventory::ChangeLootLuck(int32 Amount)
{
	constexpr uint16 MaxLootLuck = 25; // TODO expose variable
	constexpr uint16 MinLootLuck = 0; 

	// Temporarily use int32 to avoid underflow/overflow when adding negative values
	int32 NewLootLuck = static_cast<int32>(LootLuck) + Amount;

	// Clamp the result to the valid range
	NewLootLuck = FMath::Clamp(NewLootLuck, static_cast<int32>(MinLootLuck), static_cast<int32>(MaxLootLuck));

	// Assign the clamped value back to LootLuck (ensuring it's within the uint16 range)
	LootLuck = static_cast<uint16>(NewLootLuck);
}

#pragma endregion Luck

#pragma region Database

void UShibInventory::LoadInventory()
{
	if (!FishTableRef) { return; }
	
	if (UserId <= 0) return;
	
	MvAPIs->LoadInventory(UserId, [this](const TArray<FShibSavedItem>& Items)
	{
		SavedItems.Empty();
		SavedItems = Items;
	
		TArray<FShibItem> NewShibItems;
		for (auto& SavedItem : SavedItems)
		{
			auto* Row = FishTableRef->FindRow<FShibItemDataTable>(SavedItem.Name, "");
			if (!Row) continue;

			FShibItem Item = Row->CreateItem(SavedItem.Name);
			Item.Quantity = SavedItem.Quantity;
			NewShibItems.Add(Item);
		}
	
		ShibItems.Empty();
		ShibItems = NewShibItems;
	});
}

void UShibInventory::SaveInventory()
{
	if (UserId <= 0) return;
	
	FSaveInventoryRequest ItemsRequest;
	ItemsRequest.UserId = UserId;
	for (auto& Item : ShibItems)
	{
		ItemsRequest.Items.Add(FShibSavedItem{-1, Item.RowName, Item.Quantity});
	}
	
	MvAPIs->SaveInventory(ItemsRequest);
}

void UShibInventory::LoadMvPlayer()
{
	if (UserId <= 0) return;

	MvAPIs->GetMvPlayerById(UserId, [this](FShibMvPlayer InPlayer)
	{
		Player = InPlayer.Player;
	});
}

void UShibInventory::SaveMvPlayer()
{
	MvAPIs->UpdateMvPlayerById(UserId, Player);
}

#pragma endregion Database

#pragma region Replication

void UShibInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// We don't have to condition this to owner only because inventory only exists for owner and server anyway
	DOREPLIFETIME(UShibInventory, ShibItems);
	DOREPLIFETIME(UShibInventory, Player);
}

#pragma endregion Replication