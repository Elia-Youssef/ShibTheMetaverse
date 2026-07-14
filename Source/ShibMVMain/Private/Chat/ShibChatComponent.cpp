// Fill out your copyright notice in the Description page of Project Settings.


#include "Chat/ShibChatComponent.h"

#include "ShibGameChatSubsystem.h"
#include "Game/ShibGameState.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShibPlayerState.h"

UShibChatComponent::UShibChatComponent()
{
}

void UShibChatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UShibChatComponent::SendChatMessage_Implementation(const AShibPlayerState* ShibPS, const FString& Message)
{
	if (auto* GS = Cast<AShibGameState>(UGameplayStatics::GetGameState(this)))
	{
		// Since multicast RPCs don't work as expected on PlayerController, we have placed it in GameState, so the message is properly sent to all other players.
		GS->AddChatMessage(ShibPS,Message);
	}
}
