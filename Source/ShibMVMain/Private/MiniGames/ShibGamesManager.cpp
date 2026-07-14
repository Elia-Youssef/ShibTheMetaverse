// Copyright Shiba Inu Games LLC.


#include "MiniGames/ShibGamesManager.h"

#include "MiniGames/ShibGameComponent.h"
#include "Player/ShibPlayerController.h"


UShibGamesManager::UShibGamesManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UShibGamesManager::BeginPlay()
{
	Super::BeginPlay();

	ShibPC = Cast<AShibPlayerController>(GetOwner());
}

bool UShibGamesManager::StartGame(TSubclassOf<UShibGameComponent> GameComponentClass)
{
	if (GameComponentClass == nullptr)
	{
		return false;
	}
	
	// make sure we can destroy the current game
	if (!CanDestroyCurrentGame())
	{
		return false;
	}

	// create the new component and make sure it can be activated
	auto* NewGameComponent = Cast<UShibGameComponent>(
		ShibPC->AddComponentByClass(GameComponentClass, false, FTransform{}, false));
	if (!NewGameComponent->CanActivateGame())
	{
		NewGameComponent->DestroyComponent();
		return false;
	}

	// start destroying the current game component
	if (IsValid(CurrentGame) && !CurrentGame->CleanupAndDestroy())
	{
		NewGameComponent->DestroyComponent();
		return false;
	}

	// activate the new component
	if (!NewGameComponent->ActivateGame())
	{
		NewGameComponent->DestroyComponent();
		return false;
	}

	CurrentGame = NewGameComponent;
	return true;
}

bool UShibGamesManager::StopGame()
{
	// make sure we can destroy the current game
	if (!CanDestroyCurrentGame())
	{
		return false;
	}

	// start destroying the current game component
	if (IsValid(CurrentGame) && !CurrentGame->CleanupAndDestroy())
	{
		return false;
	}

	return true;
}

bool UShibGamesManager::CanDestroyCurrentGame()
{
	if (IsValid(CurrentGame))
	{
		return CurrentGame->CanDestroyGame();
	}
	return true;
}
