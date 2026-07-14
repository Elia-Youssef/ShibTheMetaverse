// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ShibBaseGameMode.h"

#include "ShibSessionsEOS.h"
#include "Game/ShibGameInstance.h"
#include "GameFramework/GameSession.h"

class UShibSessionsEOS;

AShibBaseGameMode::AShibBaseGameMode()
{
	bUseSeamlessTravel = true;
	MaxInactivePlayers = 0;
}

void AShibBaseGameMode::BeginPlay()
{
	Super::BeginPlay();

	GetShibGI();
}

void AShibBaseGameMode::KickPlayer(APlayerController* Controller, FText KickReason) const
{
	if (!GameSession || !Controller) return;
	GameSession->KickPlayer(Controller, KickReason);
}

UShibGameInstance* AShibBaseGameMode::GetShibGI()
{
	if (!ShibGI) ShibGI = GetGameInstance<UShibGameInstance>();
	return ShibGI;
}

void AShibBaseGameMode::PreLogout(APlayerController* InPlayerController)
{
	// make sure we can unregister
	if (!CanRegisterPlayer(InPlayerController)) return;
	
	const auto* UGameInstance = GetGameInstance();

	if (auto* SessionsSubsystem = UGameInstance->GetSubsystem<UShibSessionsEOS>())
	{
		if (!SessionsSubsystem->UnregisterPlayer(InPlayerController))
		{
			// The player could not be unregistered.
		}
	}
}

bool AShibBaseGameMode::CanRegisterPlayer(APlayerController* Exiting)
{
	if (Exiting->IsLocalPlayerController() && Exiting->GetLocalPlayer())
	{
		if (Exiting->GetLocalPlayer()->GetPreferredUniqueNetId() == nullptr) return false;
	}
	else
	{
		UNetConnection* RemoteNetConnection = Cast<UNetConnection>(Exiting->Player);
		if (RemoteNetConnection == nullptr) return false;
		if (RemoteNetConnection->PlayerId == nullptr) return false;
	}

	return true;
}

void AShibBaseGameMode::Logout(AController* Exiting)
{
	if(	APlayerController* PC = Cast<APlayerController>(Exiting))
	{
		// Copy pasted from LapDogs
		RemovePlayerControllerFromPlayerCount(PC);
		AddInactivePlayer(PC->PlayerState, PC);

		FGameModeEvents::GameModeLogoutEvent.Broadcast(this, Exiting);
		K2_OnLogout(Exiting);
	}

	// We completely override the logout function of the base class because we handle unregistering the player before the logout function happened
	// Super::Logout(Exiting);
}
