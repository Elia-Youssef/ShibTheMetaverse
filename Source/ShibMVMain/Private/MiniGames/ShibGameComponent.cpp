#include "MiniGames/ShibGameComponent.h"

#include "Player/ShibPlayerController.h"

UShibGameComponent::UShibGameComponent()
{
}

bool UShibGameComponent::CanActivateGame()
{
	// can activate if not already active
	return !bIsActive;
}

bool UShibGameComponent::CanDestroyGame()
{
	// destruction already in progress
	if (bPendingDestroy) return false;
	
	return bCanDestroy;
}

bool UShibGameComponent::ActivateGame()
{
	if (!CanActivateGame()) return false;
	
	bIsActive = true;
	
	return true;
}

bool UShibGameComponent::CleanupAndDestroy()
{
	if (!CanDestroyGame()) return false;
	
	bIsActive = false;
	bPendingDestroy = true;

	DestroyComponent();
	
	return false;
}

bool UShibGameComponent::CanPlayerStartGame()
{
	return true;
}

UShibInventory* UShibGameComponent::GetShibInventory() const
{
	if(const AActor* Owner = GetOwner())
	{
		if(const AShibPlayerController* ShibPlayerController = Cast<AShibPlayerController>(Owner->GetInstigatorController()))
		{
			return ShibPlayerController->GetShibInventory();
		}
	}

	return nullptr;
}

int32 UShibGameComponent::GetLootLuck() const
{
	if(const UShibInventory* ShibInventory = GetShibInventory())
	{
		return ShibInventory->GetLootLuck();
	}

	return 0;
}