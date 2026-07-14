// Copyright Shiba Inu Games LLC.

#include "Player/ShibBasePlayerController.h"
#include "ShibSessionsEOS.h"
#include "Characters/ShibCharacterBase.h"
#include "Game/ShibBaseGameMode.h"

AShibBasePlayerController::AShibBasePlayerController()
{
}

void AShibBasePlayerController::ClientWasKicked(const FText& KickReason)
{
	if (HasAuthority()) // We make sure that we don't kick the hosting player, only clients
	{
		if (!IsLocalPlayerController()) Client_WasKickedFromSession(KickReason);
	}
	else
	{
		InternalClientWasKickedFromSession(KickReason);
	}
}

void AShibBasePlayerController::Client_WasKickedFromSession_Implementation(const FText& KickReason)
{
	InternalClientWasKickedFromSession(KickReason);
}

void AShibBasePlayerController::OnNetCleanup(UNetConnection* Connection)
{
	// Handle player logout on the server side
	if (GetLocalRole() == ROLE_Authority && PlayerState != NULL)
	{
		AShibBaseGameMode* GameMode = Cast<AShibBaseGameMode>(GetWorld()->GetAuthGameMode());
		if (IsValid(GameMode))
		{
			GameMode->PreLogout(this);
		}
	}

	Super::OnNetCleanup(Connection);
}

void AShibBasePlayerController::FindAndJoinSession(UOnlineSessionSearch* SearchSettings)
{
	InternalFindAndJoinSession(SearchSettings);
	
	auto* GameInstance = GetGameInstance();
	if (auto* SessionSubsystem = GameInstance->GetSubsystem<UShibSessionsEOS>())
	{
		SessionSubsystem->OnFindSessionsComplete.AddDynamic(this, &AShibBasePlayerController::AShibBasePlayerController::OnSessionFoundHandle);
		SessionSubsystem->FindSessions(SearchSettings);
	}
}

void AShibBasePlayerController::CancelFindSessions()
{
	auto* GameInstance = GetGameInstance();
	if (auto* SessionSubsystem = GameInstance->GetSubsystem<UShibSessionsEOS>())
	{
		SessionSubsystem->OnFindSessionsComplete.RemoveAll(this);
		SessionSubsystem->OnJoinSessionComplete.RemoveAll(this);

		InternalOnCancelFindSessions();

		SessionSubsystem->OnCancelFindSessionsComplete.AddDynamic(this, &AShibBasePlayerController::AShibBasePlayerController::OnCancelFindSessionsHandle);
		SessionSubsystem->CancelFindSessions();
	}
}

void AShibBasePlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	if (auto* ShibPawn = Cast<AShibCharacterBase>(GetPawn()))
	{
		ShibPawn->OnReplicationReady();
	}
}

void AShibBasePlayerController::ClientTravelToSession(const FString Address)
{
	ClientTravel(Address, TRAVEL_Absolute);
}

void AShibBasePlayerController::OnSessionFoundHandle(const TArray<FOnlineSessionSearchResultBP>& SessionResults, bool bWasSuccessful)
{
	auto* GameInstance = GetGameInstance();
	if (auto* SessionSubsystem = GameInstance->GetSubsystem<UShibSessionsEOS>())
	{
		SessionSubsystem->OnFindSessionsComplete.RemoveAll(this);

		// If we found valid session, we bind the OnJoinSession delegate right away
		// because we will probably try to join one right after this
		if (bWasSuccessful && !SessionSubsystem->OnJoinSessionComplete.IsAlreadyBound(this, &AShibBasePlayerController::AShibBasePlayerController::OnSessionJoinedHandle))
		{
			SessionSubsystem->OnJoinSessionComplete.AddDynamic(this, &AShibBasePlayerController::AShibBasePlayerController::OnSessionJoinedHandle);
		}
		
		InternalOnSessionFound(SessionResults, bWasSuccessful);
	}
}

void AShibBasePlayerController::OnSessionJoinedHandle(bool bWasSuccessful, const EShibJoinSessionResultTypeEOS Type, const FString& Address)
{
	auto* GameInstance = GetGameInstance();
	if (auto* SessionSubsystem = GameInstance->GetSubsystem<UShibSessionsEOS>())
	{
		SessionSubsystem->OnJoinSessionComplete.RemoveAll(this);
	}

	InternalOnSessionJoined(bWasSuccessful, Type, Address);
}

void AShibBasePlayerController::OnCancelFindSessionsHandle(bool bWasSuccessful)
{
	auto* GameInstance = GetGameInstance();
	if (auto* SessionSubsystem = GameInstance->GetSubsystem<UShibSessionsEOS>())
	{
		SessionSubsystem->OnCancelFindSessionsComplete.RemoveAll(this);
	}
}
