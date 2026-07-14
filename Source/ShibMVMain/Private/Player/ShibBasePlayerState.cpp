// Copyright Shiba Inu Games LLC.

#include "Player/ShibBasePlayerState.h"

AShibBasePlayerState::AShibBasePlayerState()
{
	bIsLeavingSession = false;
	bReplicates = true;
}

void AShibBasePlayerState::OnDeactivated()
{
	bIsReconnecting = false;
    
	if (const APawn* MyPawn = GetPawn<APawn>())
	{
		DisconnectedPlayerData.PlayerPosition = MyPawn->GetTransform();
	}

	Super::OnDeactivated();
}

void AShibBasePlayerState::OnReactivated()
{
	bIsReconnecting = true;
}

void AShibBasePlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
}

void AShibBasePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
