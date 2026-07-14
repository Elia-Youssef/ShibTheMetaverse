// Copyright Shiba Inu Games LLC.


#include "MiniGames/Fishing/FishingComponent.h"

#include "Characters/ShibCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/ShibPlayerController.h"

UFishingComponent::UFishingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFishingComponent::AttemptToCatchFish()
{
	// Only runs on server
	if( ! FishingIndicator)
	{
		// If fish is already caught, stop fishing
		StopFishing();
		return;
	}
	
	if (const UWorld* World = GetWorld())
	{
		if (FishToCatch.ItemIsValid())
		{
			if (AActor* Owner = GetOwner())
			{
				if (AShibCharacterBase* ShibCharacter = Cast<AShibCharacterBase>(Owner))
				{
					ShibCharacter->SetAnimationState(AS_FishingEnd);
				
					if (AShibPlayerController* ShibController = Cast<AShibPlayerController>(ShibCharacter->GetController()))
					{
						if(UShibInventory* ShibInventory = ShibController->GetShibInventory())
						{
							ShibInventory->AddItemFromStruct(FishToCatch);

							Client_ItemPopUp(FishToCatch.RowName);
							//TODO InRate determines how long before stopping fishing. This is so the ending animation can play out
							World->GetTimerManager().SetTimer(Timer_Fishing, this, &UFishingComponent::StopFishing, 5.f, false);
							
							ShibInventory->AdjustLootLuck(FishToCatch.ItemTier);
						}
						
						// RemoveFishingIndicator();
					}
				}
			}
		}
		else
		{
			StopFishing();
		}
	}

	FishToCatch.ItemName = FName();
}

bool UFishingComponent::CanPlayerStartGame()
{
	return true;

	//TODO enable later
	// if (UShibInventory* ShibInventory = GetShibInventory())
	// {
	// 	if (ShibInventory->RemoveItemByName(ItemToConsume, 1))
	// 	{
	// 		return true; // Checks if inventory contains necessary item to fish
	// 	}
	// }
	//
	// return false;
}

void UFishingComponent::InitializeFishingComponent(FVector HookPoint)
{
	if( ! CanPlayerStartGame())
	{
		DestroyComponent(false);
		return;
	}

	SetIsReplicated(true);
	
	if (const UWorld* World = GetWorld())
	{
		const float RandomTime = FMath::RandRange(MinWaitingTime, MaxWaitingTime); // Random time between 2 and 10 seconds
		World->GetTimerManager().SetTimer(Timer_Fishing, this, &UFishingComponent::FishHooked, RandomTime, false);
	}

	FishingLocation = HookPoint;
	OnRep_FishingLocation();

	if (AActor* Owner = GetOwner())
	{
		if (AShibCharacterBase* ShibCharacter = Cast<AShibCharacterBase>(Owner))
		{
			ShibCharacter->SetAnimationState(AS_FishingStart);
		}
	}
}

void UFishingComponent::OnCharacterMoved()
{
	StopFishing();
}


void UFishingComponent::DestroyComponent(bool bPromoteChildren)
{
	RemoveFishingIndicator();

	Super::DestroyComponent(bPromoteChildren);
}

void UFishingComponent::OnRep_FishToCatch()
{
	if ( ! FishToCatch.ItemName.IsNone())
	{
		if (FishingIndicator)
		{
			FishingIndicator->OnFishHooked();
		}
	}
	else
	{
		RemoveFishingIndicator();
	}
}

void UFishingComponent::OnRep_FishingLocation()
{
	if (AFishingBobber* NewFishingIndicator = SpawnFishingHookPointAtLocation(FishingLocation))
	{
		FishingIndicator = NewFishingIndicator;
		FishingIndicator->SetMaxWaitingTime(MaxWaitingTime);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn hooking actor."));
	}
}

void UFishingComponent::FishHooked()
{
	// Sets the fish which will trigger an OnRep function
	FishToCatch = GetRandomFishToCatch();
	
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(Timer_Fishing, this, &UFishingComponent::StopFishing, MinWaitingTime, false);
	}
}

void UFishingComponent::RemoveFishingIndicator()
{
	if (FishingIndicator)
	{
		FishingIndicator->Destroy();
		FishingIndicator = nullptr;
	}
}

void UFishingComponent::Client_ItemPopUp_Implementation(FName RowName)
{
	// UE_LOG(LogTemp, Warning, TEXT("Client popped up a fish: %s"), *RowName.ToString());
	OnShibItemPopUp.Broadcast(RowName);
}

void UFishingComponent::StopFishing()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(Timer_Fishing);
	}

	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(Timer_Fishing, this, &UFishingComponent::FishHooked, FishingEndingDuration, false);
	}

	DestroyComponent(false);
}

void UFishingComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (AActor* Owner = GetOwner())
	{
		if (const ACharacter* Character = Cast<ACharacter>(Owner))
		{
			Character->GetCharacterMovement()->SetMovementMode(MOVE_None); // Disable movement
		}
	}
}

void UFishingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AActor* Owner = GetOwner())
	{
		if (AShibCharacterBase* ShibCharacter = Cast<AShibCharacterBase>(Owner))
		{
			ShibCharacter->SetAnimationState(AS_None);
			ShibCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking); // Enable movement
		}
	}

	Super::EndPlay(EndPlayReason);
}

AFishingBobber* UFishingComponent::SpawnFishingHookPointAtLocation(const FVector& HookPoint) const
{
	if (FishingHookClass)
	{
		if (UWorld* World = GetWorld())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = GetOwner()->GetInstigator();

			AFishingBobber* SpawnedActor = World->SpawnActor<AFishingBobber>(FishingHookClass, HookPoint, FRotator::ZeroRotator,
			                                                 SpawnParams);
			return SpawnedActor;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load FishingHookClass"));
	}

	return nullptr;
}

void UFishingComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UFishingComponent, FishToCatch);
	DOREPLIFETIME_CONDITION(UFishingComponent, FishingLocation, COND_InitialOnly);
}

FShibItem UFishingComponent::GetRandomFishToCatch() const
{
	FShibItem Fish = GetRandomFish();
	Fish.Quantity = 1;
	return Fish;
}

FShibItem UFishingComponent::GetRandomFish() const
{
	TArray<FName> ValidFishRowNames;
	const UDataTable* FishDataTable = GetShibInventory()->GetDataTableByCategory(IC_Fishing);
    
	if ( ! FishDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("FishDataTable is nullptr in GetRandomFish"));
		return FShibItem();
	}

	// Find all valid fish row names
	for (const FName& RowName : FishDataTable->GetRowNames())
	{
		static const FString ContextString(TEXT("GetRandomFish Context"));
		const FShibItemDataTable* ItemData = FishDataTable->FindRow<FShibItemDataTable>(RowName, ContextString);

		// Select an item from the data table that is a certain tier and has the correct sub category
		const EShibItemTier RandomTier = DetermineFishTier();
		if (ItemData && ItemData->ItemSubCategory == ISC_RawFish && ItemData->ItemTier == RandomTier)
		{
			ValidFishRowNames.Add(RowName);
		}
	}

	// If no valid fish are found, return an empty item
	if (ValidFishRowNames.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid fish items found in GetRandomFish"));
		return FShibItem();
	}
	
	// Select a random fish row name from valid items
	const int32 RandomIndex = FMath::RandRange(0, ValidFishRowNames.Num() - 1);
	const FName SelectedRowName = ValidFishRowNames[RandomIndex];

	// Fetch the selected fish data
	static const FString ContextString(TEXT("GetRandomFish Context"));
	const FShibItemDataTable* SelectedFish = FishDataTable->FindRow<FShibItemDataTable>(SelectedRowName, ContextString);
    
	if ( ! SelectedFish)
	{
		UE_LOG(LogTemp, Warning, TEXT("Selected fish data is nullptr in GetRandomFish"));
		return FShibItem();
	}

	// Create the item with the correct RowName as an ID
	const FShibItem RandomFish = SelectedFish->CreateItem(SelectedRowName);
	
	return RandomFish;
}

EShibItemTier UFishingComponent::DetermineFishTier() const
{
	// Rolls are influenced by "Loot Luck" which is changed depending on what tier items have been found recently
	const int32 AdjustedRoll = FMath::Clamp(FMath::RandRange(1, 100) - GetLootLuck(), 1, 100);

	if (AdjustedRoll <= EpicFishChance)
		return IT_Epic;
	if (AdjustedRoll <= EpicFishChance + RareFishChance)
		return IT_Rare;
	if (AdjustedRoll <= EpicFishChance + RareFishChance + UncommonFishChance)
		return IT_Uncommon;
    
	return IT_Common;
}

