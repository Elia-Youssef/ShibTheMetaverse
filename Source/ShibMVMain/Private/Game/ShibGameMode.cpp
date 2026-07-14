// Copyright Shib LLC.

#include "Game/ShibGameMode.h"

#include "OnlineSubsystemUtils.h"
#include "ShibSessionsEOS.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"
#include "Player/ShibPlayerController.h"
#include "Player/OnlineVoiceAdminInterface.h"

DEFINE_LOG_CATEGORY(LogShibGameMode);

void AShibGameMode::BeginPlay()
{
	Super::BeginPlay();
	
}

void AShibGameMode::Logout(AController* Exiting)
{
	APlayerController* PC = Cast<APlayerController>(Exiting);
	if (!IsValid(PC) || !IsValid(PC->Player))
	{
		Super::Logout(Exiting);
		return;
	}
	
	if(GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		if (!PC->IsLocalPlayerController())
		{
			UE_LOG(LogShibGameMode,Error,TEXT("%hs: Player Controller Not Local"),__FUNCTION__)
			Super::Logout(Exiting);
			return;
		}
	
		UNetConnection* PCNet = Cast<UNetConnection>(PC->Player);
		if (!IsValid(PCNet) || !PCNet->PlayerId.IsValid())
		{
			UE_LOG(LogShibGameMode,Error,TEXT("%hs: PlayerID Not Valid "),__FUNCTION__)
			Super::Logout(Exiting);
			return;
		}

		TSharedRef<const FUniqueNetId> UserId = PCNet->PlayerId.GetUniqueNetId().ToSharedRef();

		IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
		const auto Identity = Subsystem->GetIdentityInterface();
		const auto VoiceAdmin = Online::GetVoiceAdminInterface(Subsystem);

		// You'll need to use the same channel as when you issued credentials in the first place.
		VoiceAdmin->KickParticipant(
			*Identity->GetUniquePlayerId(0), // This will get the "dedicated server" ID.
			PublicVoiceChannelName, // Ideally each session should have its own channel
			*UserId,
			FOnVoiceAdminKickParticipantComplete::CreateLambda([](
				const FOnlineError& Result,
				const FUniqueNetId& LocalUserId,
				const FUniqueNetId& TargetUserId)
			{
				if (!Result.WasSuccessful())
				{
					UE_LOG(LogShibGameMode,Error,TEXT("%hs: Unable to kick player"),__FUNCTION__)
				}
			}));
	}
	
	Super::Logout(Exiting);
}

void AShibGameMode::OnPostLogin(AController* NewPlayer)
{
	auto* SessionsSubsystem = GetGameInstance()->GetSubsystem<UShibSessionsEOS>();
	auto* ShibCtrl = Cast<AShibPlayerController>(NewPlayer);

	if (!ShibCtrl || !SessionsSubsystem) return;

	// make sure we can register
	if (!CanRegisterPlayer(ShibCtrl)) return;
	
	SessionsSubsystem->RegisterPlayer(ShibCtrl);
	Super::OnPostLogin(NewPlayer);
}

void AShibGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (auto* ShibPlayerController = Cast<AShibPlayerController>(NewPlayer))
	{
		// running in the controller's beginplay for the demo
		// ShibPlayerController->Client_GetTeleportLocation();
	}
}

void AShibGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);
	
	if (auto* ShibPlayerController = Cast<AShibPlayerController>(C))
	{
		// running in the controller's beginplay for the demo
		// ShibPlayerController->Client_GetTeleportLocation();
	}
}

void AShibGameMode::RestartPlayer(AController* NewPlayer)
{
	if(AShibPlayerController* ShibPlayerController = Cast<AShibPlayerController>(NewPlayer))
	{
		// Does not allow possession until the teleportation tag is confirmed
		if(ShibPlayerController->bTagHasBeenConfirmed)
		{
			Super::RestartPlayer(NewPlayer);
		}
	}
}

void AShibGameMode::IssuedCredentialsForPlayer(const FOnlineError& Result, const FUniqueNetId& LocalUserId,
                                               const TArray<FVoiceAdminChannelCredentials>& Credentials,
                                               FString InChannelName, AShibPlayerController* NewPlayer)
{
	if (!Result.WasSuccessful())
	{
		UE_LOG(LogShibGameMode,Error,TEXT("%hs: Failed to issue channel credentials"),__FUNCTION__);
		return;
	}
	
	// Logging everything to check if it is correct
	UE_LOG(LogShibGameMode, Log, TEXT("%hs: Result: %d"),__FUNCTION__, Result.bSucceeded);
	UE_LOG(LogShibGameMode, Log, TEXT("%hs: LocalUserId: %s"),__FUNCTION__, *LocalUserId.ToString());
	UE_LOG(LogShibGameMode, Log, TEXT("%hs: Credential: %s"),__FUNCTION__, *Credentials[0].ChannelCredentials);
	UE_LOG(LogShibGameMode, Log, TEXT("%hs: InChannelName: %s"),__FUNCTION__, *InChannelName);
	
	if (IsValid(NewPlayer))
	{
		UE_LOG(LogShibGameMode, Log, TEXT("%hs: NewPlayer= %s"),__FUNCTION__, *NewPlayer->GetName());
	}
	else
	{
		UE_LOG(LogShibGameMode, Log, TEXT("%hs: NewPlayer = nullptr"),__FUNCTION__);
	}

	// We're making one CreateChannelCredentials call per player, so we can assume that the only entry in the
	// results is for the player this callback is associated with. If you were requesting credentials for multiple
	// target players, you'd have to iterate through Credentials to find the matching "TargetUserId" field.
	if (IsValid(NewPlayer))
	{
		UE_LOG(LogShibGameMode,Log,TEXT("%hs: Client RPC to Join Voice Channel"),__FUNCTION__);
		NewPlayer->JoinVoiceChannel(InChannelName, Credentials[0].ChannelCredentials);
	}
}

void AShibGameMode::AutoJoinPlayerToVoiceChannel(APlayerController* NewPlayer)
{
	if (GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		if(!NewPlayer->IsLocalController())
		{
			AShibPlayerController * CustomPC=Cast<AShibPlayerController>(NewPlayer);
			if (IsValid(CustomPC))
			{
				
				UNetConnection* IncomingNetConnection = Cast<UNetConnection>(CustomPC->Player);

				if (!IsValid(IncomingNetConnection))
				{
					UE_LOG(LogShibGameMode,Error,TEXT("%hs: Incoming Net Connection Invalid"),__FUNCTION__)
					return;
				}
				if (!IncomingNetConnection->PlayerId.IsValid())
				{
					UE_LOG(LogShibGameMode,Error,TEXT("%hs: Incoming Net Connection PlayerID Not Valid"),__FUNCTION__)
					return;
				}

				UE_LOG(LogShibGameMode,Log,TEXT("%hs: Creating Channel Credentials"),__FUNCTION__);

				IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
				TSharedPtr<IOnlineIdentity, ESPMode::ThreadSafe> Identity = Subsystem->GetIdentityInterface();
				TSharedPtr<IOnlineVoiceAdmin, ESPMode::ThreadSafe> VoiceAdmin = Online::GetVoiceAdminInterface(Subsystem);

				// For the channel name you could use the current session ID. It's up to you
				// to choose an appropriate value based on how you want to group players in
				// voice chat.

				TArray<TSharedRef<const FUniqueNetId>> TargetUserIds;
				TargetUserIds.Add(IncomingNetConnection->PlayerId.GetUniqueNetId().ToSharedRef());
				VoiceAdmin->CreateChannelCredentials(
					*Identity->GetUniquePlayerId(0), // This will get the "dedicated server" ID.
					PublicVoiceChannelName,
					TargetUserIds,
					FOnVoiceAdminCreateChannelCredentialsComplete::CreateUObject(
						this, 
						&AShibGameMode::IssuedCredentialsForPlayer, 
						PublicVoiceChannelName,
						CustomPC));
			}
			else
			{
				UE_LOG(LogShibGameMode,Error,TEXT("%hs: Invalid PlayerController"),__FUNCTION__)
			}
		}
	}
	else
	{
		UE_LOG(LogShibGameMode,Warning,TEXT("%hs: Not Running Dedicated Server"),__FUNCTION__)
	}
}

void AShibGameMode::NotifyClientsOfNewPlayer(FUniqueNetIdPtr NewPlayerID)
{
	// Iterating through all player controllers and muting this player for them, as soon as this player joins the channel
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AShibPlayerController* PlayerController = Cast<AShibPlayerController>(*It))
		{
			if (NewPlayerID != PlayerController->PlayerState->GetUniqueId().GetUniqueNetId())
			{
				PlayerController->OnNewPlayerLoggedIn(NewPlayerID);
			}
		}
	}
}

void AShibGameMode::UnregisterPlayer(APlayerController* InPlayerController)
{
	// make sure we can register
	if (!CanRegisterPlayer(InPlayerController)) return;

	auto* SessionsSubsystem = GetGameInstance()->GetSubsystem<UShibSessionsEOS>();

	if (SessionsSubsystem && InPlayerController->GetLocalPlayer()->GetPreferredUniqueNetId() != nullptr)
	{
		if (!SessionsSubsystem->UnregisterPlayer(InPlayerController))
		{
			// The player could not be unregistered.
		}
	}
}
