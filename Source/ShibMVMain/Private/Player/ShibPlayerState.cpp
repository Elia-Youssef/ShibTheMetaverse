// Copyright Shiba Inu Games LLC.

#include "Player/ShibPlayerState.h"
#include "Net/UnrealNetwork.h"

AShibPlayerState::AShibPlayerState()
{

}

void AShibPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void AShibPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AShibPlayerState, Player, COND_OwnerOnly);
}

void AShibPlayerState::OnRep_Player()
{
	// Update UI or something
}
