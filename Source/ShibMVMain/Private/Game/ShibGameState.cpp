// Copyright Shiba Inu Games LLC.

#include "Game/ShibGameState.h"
#include "ShibGameChatSubsystem.h"
#include "ShibSessionsEOS.h"
#include "Characters/ShibCharacterBase.h"
#include "GameFramework/PlayerState.h"

AShibGameState::AShibGameState()
{
}

void AShibGameState::BeginPlay()
{
	Super::BeginPlay();
}

void AShibGameState::AddPlayerState(APlayerState* PlayerState)
{
	Internal_OnPlayerJoin(PlayerState);
	Super::AddPlayerState(PlayerState);
}

void AShibGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Internal_OnPlayerLeave(PlayerState);
	Super::RemovePlayerState(PlayerState);

	if(PlayerArray.IsEmpty())
	{
		const auto* UGameInstance = GetGameInstance();

		if (auto* SessionsSubsystem = UGameInstance->GetSubsystem<UShibSessionsEOS>())
		{
			// Terminate session if there are no players
			SessionsSubsystem->DestroySession();
		}
	}
}

void AShibGameState::Internal_OnPlayerJoin_Implementation(APlayerState* InPlayerState)
{
	if (IsValid(InPlayerState) && OnPlayerArrayChangeDelegate.IsBound())
	{
		OnPlayerArrayChangeDelegate.Broadcast(InPlayerState);
	}
}

void AShibGameState::Internal_OnPlayerLeave_Implementation(APlayerState* InPlayerState)
{
	if (IsValid(InPlayerState) && OnPlayerArrayChangeDelegate.IsBound())
	{
		OnPlayerArrayChangeDelegate.Broadcast(InPlayerState);
	}
}

void AShibGameState::AddChatMessage_Implementation(const AShibPlayerState* ShibPS, const FString& Message)
{
	UShibGameChatSubsystem * ChatSubsystem=GetWorld()->GetGameInstance()->GetSubsystem<UShibGameChatSubsystem>();
	ChatSubsystem->SendMessage(ShibPS->GetPlayerName(),Message);
}

TArray<FVector> AShibGameState::GetAllPlayerLocations()
{
	TArray<FVector> PlayerLocations;
	for (APlayerState* PlayerState : PlayerArray)
	{
		AShibCharacterBase* Player = Cast<AShibCharacterBase>(PlayerState->GetPawn());
		if (Player)
		{
			PlayerLocations.Add(Player->GetActorLocation());
		}
	}
	return PlayerLocations;
}