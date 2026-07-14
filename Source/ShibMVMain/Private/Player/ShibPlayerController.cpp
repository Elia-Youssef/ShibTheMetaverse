// Copyright Shib LLC.

#include "Player/ShibPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Game/ShibGameMode.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "ShibUserEOS.h"
#include "VoiceChat.h"
#include "Actor/TravelPod.h"
#include "Characters/ShibCharacterBase.h"
#include "Game/ShibGameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Metaverse/MetaverseApisSubsystem.h"
#include "Player/ShibPlayerState.h"
#include "Plots/BasePlot.h"

DEFINE_LOG_CATEGORY(LogShibPlayerController);

AShibPlayerController::AShibPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AShibPlayerController::GetAudioListenerPosition(FVector& OutLocation, FVector& OutFrontDir,
                                                     FVector& OutRightDir) const
{
	// We completely override the function from base class
	//Super::GetAudioListenerPosition(OutLocation, OutFrontDir, OutRightDir);

	FVector ViewLocation;
	FRotator ViewRotation;

	if (bOverrideAudioListener) // This is the same as the base class
	{
		USceneComponent* ListenerComponent = AudioListenerComponent.Get();
		if (ListenerComponent != nullptr)
		{
			ViewRotation = ListenerComponent->GetComponentRotation() + AudioListenerRotationOverride;
			ViewLocation = ListenerComponent->GetComponentLocation() + ViewRotation.RotateVector(
				AudioListenerLocationOverride);
		}
		else
		{
			ViewLocation = AudioListenerLocationOverride;
			ViewRotation = AudioListenerRotationOverride;
		}
	}
	else
	// This is where we change the view location to the pawn location instead of the Player controller (camera location)
	{
		GetPlayerViewPoint(ViewLocation, ViewRotation);
		if (GetPawn())
		{
			ViewLocation = GetPawn()->GetActorLocation();
		}
	}

	const FRotationTranslationMatrix ViewRotationMatrix(ViewRotation, ViewLocation);

	OutLocation = ViewLocation;
	OutFrontDir = ViewRotationMatrix.GetUnitAxis(EAxis::X);
	OutRightDir = ViewRotationMatrix.GetUnitAxis(EAxis::Y);
}

#pragma region Construction/Destruction

void AShibPlayerController::BeginReplication()
{
	Super::BeginReplication();

	if (!GetGameInstance()) return;

	UMetaverseApisSubsystem* APIs = GetGameInstance()->GetSubsystem<UMetaverseApisSubsystem>();
	if (!APIs) return;

	InitializeInventory(APIs->MvPlayer.User.UserId);
}

void AShibPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// demo
	Client_GetTeleportLocation();
}

void AShibPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (VoiceChat && VoiceChatUser) VoiceChat->ReleaseUser(VoiceChatUser);

	OnPreTeleportToLocation.RemoveAll(this);
	OnPostTeleportToLocation.RemoveAll(this);
}

void AShibPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (HasAuthority())
	{
		if (AShibCharacterBase* ShibCharacter = Cast<AShibCharacterBase>(InPawn))
		{
			OnVoiceChatToggle.AddDynamic(ShibCharacter, &AShibCharacterBase::TogglePlayerTalking);
		}
	}
}

void AShibPlayerController::ClientTravelToSession(const FString Address)
{
	LeaveAllChannels();
	Super::ClientTravelToSession(Address);
}

void AShibPlayerController::ToggleMappingContexts(bool bEnabled, TArray<UInputMappingContext*> MappingContexts)
{
	AShibCharacterBase* ShibChar = Cast<AShibCharacterBase>(GetPawn());
	if (!ShibChar) return;

	if (auto* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(this->GetLocalPlayer()))
	{
		for (auto Mapping : MappingContexts)
		{
			if (bEnabled)
			{
				if (!InputSubsystem->HasMappingContext(Mapping))
				{
					InputSubsystem->AddMappingContext(Mapping, 0);
				}
			}
			else
			{
				if (InputSubsystem->HasMappingContext(Mapping))
				{
					InputSubsystem->RemoveMappingContext(Mapping);
				}
			}
		}
	}
}

#pragma endregion Construction/Destruction

#pragma region Teleport

void AShibPlayerController::Server_TeleportToLocation(const FName& PodTag)
{
	// A rough check to confirm the TAG to allow possessing the pawn
	bTagHasBeenConfirmed = true;
	ServerRestartPlayer();

	TArray<AActor*> Pods;
	UGameplayStatics::GetAllActorsOfClassWithTag(this, ATravelPod::StaticClass(), PodTag, Pods);

	// if there are no pod actors, check for child components in plots
	if (Pods.IsEmpty())
	{
		TArray<AActor*> Plots;
		UGameplayStatics::GetAllActorsOfClassWithTag(this, ABasePlot::StaticClass(), PodTag, Plots);
		if (!Plots.IsEmpty())
		{
			TArray<UActorComponent*> Components = Plots[0]->GetComponentsByTag(
				UChildActorComponent::StaticClass(), PodTag);
			if (!Components.IsEmpty())
			{
				if (const auto* ChildComponent = Cast<UChildActorComponent>(Components[0]))
				{
					if (GetPawn()->SetActorLocation(
						ChildComponent->GetComponentLocation() + RelativeLocationWhenTeleporting,
						false, nullptr, ETeleportType::TeleportPhysics))
					{
						FRotator ActorRot = GetPawn()->GetActorRotation();
						ActorRot.Yaw = ChildComponent->GetComponentRotation().Yaw;
						GetPawn()->SetActorRotation(ActorRot);
						Client_OnPostTeleportToLocationHandle(true);
					}
					return;
				}
			}
		}
	}

	// if we still find nothing, check for default pods
	if (Pods.IsEmpty() && PodTag != FName("Default"))
		UGameplayStatics::GetAllActorsOfClassWithTag(this, ATravelPod::StaticClass(), FName("Default"), Pods);

	if (!Pods.IsEmpty())
	{
		const AActor* Pod = Pods[FMath::RandRange(0, Pods.Num() - 1)];
		if (Pod && GetPawn())
		{
			if (GetPawn()->SetActorLocation(Pod->GetActorLocation() + RelativeLocationWhenTeleporting,
			                                false, nullptr, ETeleportType::TeleportPhysics))
			{
				FRotator ActorRot = GetPawn()->GetActorRotation();
				ActorRot.Yaw = Pod->GetActorRotation().Yaw;
				GetPawn()->SetActorRotation(ActorRot);
				Client_OnPostTeleportToLocationHandle(true);
			}
		}
	}

	Client_OnPostTeleportToLocationHandle(false);
}

void AShibPlayerController::Client_OnPostTeleportToLocationHandle(bool bTeleportSuccess)
{
	// Notify everyone that we got teleported successfully
	OnPostTeleportToLocation.Broadcast(bTeleportSuccess);
}

void AShibPlayerController::Client_GetTeleportLocation()
{
	if (const auto* GameInstance = GetGameInstance<UShibGameInstance>())
	{
		Server_TeleportToLocation(GameInstance->TeleporterTag);
		return;
	}

	Server_TeleportToLocation(FName("Default"));
}

#pragma endregion Teleport

#pragma region Inventory

void AShibPlayerController::InitializeInventory(int32 UserId)
{
	if (UActorComponent* FoundComponent = GetComponentByClass(UShibInventory::StaticClass()))
	{
		if (UShibInventory* ShibInventory = Cast<UShibInventory>(FoundComponent))
		{
			ShibInventoryComponent = ShibInventory;
			ShibInventoryComponent->Server_InitializeInventory(UserId);

			// Possibly a redundant condition check but it helps me sleep at night
			if (!ShibInventoryComponent->OnShibItemsUpdated.IsAlreadyBound(
				this, &AShibPlayerController::HandleInventoryUpdate))
			{
				ShibInventoryComponent->OnShibItemsUpdated.AddDynamic(
					this, &AShibPlayerController::HandleInventoryUpdate);
			}
		}
	}
}

TArray<FShibItem> AShibPlayerController::GetShibItems() const
{
	if (ShibInventoryComponent)
	{
		return ShibInventoryComponent->GetItems();
	}

	TArray<FShibItem> DefaultItems;
	return DefaultItems;
}

UShibInventory* AShibPlayerController::GetShibInventory() const
{
	return ShibInventoryComponent;
}

void AShibPlayerController::HandleInventoryUpdate()
{
	OnInventoryUpdated(); // Blueprint event
}

#pragma endregion Inventory

#pragma region VoiceChat

void AShibPlayerController::JoinVoiceChannel(const FString& InChannelName,
                                                            const FString& InChannelCredentials)
{
	if (this->VoiceChatUser == nullptr)
	{
		UE_LOG(LogShibPlayerController, Error, TEXT("%hs: VoiceChat User is null"), __FUNCTION__);
		return;
	}

	// Creating a variable PC, so we can use our PlayerController in the lambda function
	AShibPlayerController* PC = this;

	this->VoiceChatUser->JoinChannel(InChannelName, InChannelCredentials, EVoiceChatChannelType::NonPositional,
	                                 FOnVoiceChatChannelJoinCompleteDelegate::CreateLambda(
		                                 [PC](const FString& ChannelName, const FVoiceChatResult& Result)
		                                 {
			                                 if (Result.IsSuccess())
			                                 {
				                                 // Here we tell the server that this player has joined the voice channel, and all other players should mute this player.
				                                 PC->MuteSelf(true);
				                                 PC->ServerNotifyChannelJoin(PC);
				                                 GEngine->AddOnScreenDebugMessage(
					                                 -1, 20, FColor::Red,
					                                 FString::Printf(TEXT("Successfully joined channel")));
			                                 }
			                                 else
			                                 {
				                                 UE_LOG(LogShibPlayerController, Error,
				                                        TEXT("%hs: Unsuccessful channel join attempt - %s"),
				                                        __FUNCTION__, *Result.ErrorDesc);
			                                 }
		                                 }));
}

void AShibPlayerController::OnNewPlayerLoggedIn(FUniqueNetIdPtr RemotePlayerID)
{
	MuteRemotePlayer(RemotePlayerID);
}

void AShibPlayerController::LeaveAllChannels()
{
	if (VoiceChatUser)
	{
		// Store in a variable because we don't want to modify the array we're looping on
		TArray<FString> Channels = VoiceChatUser->GetChannels();
		for (auto& Channel : Channels)
		{
			UE_LOG(LogShibPlayerController, Log, TEXT("%hs: Leaving Channel `%s`"), __FUNCTION__, *Channel);
			VoiceChatUser->LeaveChannel(Channel, nullptr);
		}
	}
}

void AShibPlayerController::InitVoiceChat()
{
	if (!IsLocalPlayerController()) return;

	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	if (!Subsystem) return;

	IOnlineIdentityPtr IdentityInterface = Subsystem->GetIdentityInterface();

	auto* UserSubsystem = GetGameInstance()->GetSubsystem<UShibUserEOS>();
	if (!UserSubsystem || !UserSubsystem->IsPlayerLoggedIn()) return;

	if (!IsValid(PlayerState)) return;

	VoiceChat = IVoiceChat::Get();

	VoiceChatUser = VoiceChat->CreateUser();
	const TSharedPtr<const FUniqueNetId> UserId = IdentityInterface->GetUniquePlayerId(0);
	const FPlatformUserId PlatformUserId = IdentityInterface->GetPlatformUserIdFromUniqueNetId(*UserId);

	VoiceChatUser->Login(PlatformUserId, PlayerState->GetUniqueId().GetUniqueNetId()->ToString(), TEXT(""),
	                     FOnVoiceChatLoginCompleteDelegate::CreateUObject(this, &ThisClass::OnVoiceLoginComplete));
}

void AShibPlayerController::ServerNotifyLogin(APlayerController* NewPlayer)
{
	UE_LOG(LogShibPlayerController, Log, TEXT("%hs: Joining Player to Voice Channel"), __FUNCTION__);

	AShibGameMode* GameMode = Cast<AShibGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->AutoJoinPlayerToVoiceChannel(NewPlayer);
	}
}

void AShibPlayerController::ServerNotifyChannelJoin(AShibPlayerController* NewPlayer)
{
	UE_LOG(LogShibPlayerController, Log, TEXT("%hs: Muting Player %d for other players"), __FUNCTION__,
	       NewPlayer->GetUniqueID());

	AShibGameMode* GameMode = Cast<AShibGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->NotifyClientsOfNewPlayer(PlayerState->GetUniqueId().GetUniqueNetId());
	}
}

void AShibPlayerController::SetVoiceInputVolume(const float Volume)
{
	if (!VoiceChatUser) return;

	InputVolume = FMath::Clamp(Volume, 0.f, 200.f);
	VoiceChatUser->SetAudioInputVolume(InputVolume);
}

void AShibPlayerController::SetVoiceOutputVolume(const float Volume)
{
	if (!VoiceChatUser) return;

	OutputVolume = FMath::Clamp(Volume, 0.f, 200.f);
	VoiceChatUser->SetAudioOutputVolume(OutputVolume);
}

void AShibPlayerController::MuteSelf(bool Mute)
{
	Server_ToggleVoiceChat(!Mute); // Placed before the check so I can test in editor
	if (!VoiceChatUser) return;

	if (Mute)
	{
		VoiceChatUser->SetAudioInputVolume(0);

		UE_LOG(LogShibPlayerController, Log, TEXT("%hs - Muted Player"), __FUNCTION__);
	}
	else
	{
		VoiceChatUser->SetAudioInputVolume(InputVolume);

		UE_LOG(LogShibPlayerController, Log, TEXT("%hs - Set input voice volume to : %f "), __FUNCTION__, InputVolume);
	}
}

void AShibPlayerController::OnVoiceLoginComplete(const FString& PlayerName, const FVoiceChatResult& Result)
{
	GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Purple,
	                                 FString::Printf(TEXT("Voice logged in attempted for - %s"), *PlayerName));
	GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Purple,
	                                 FString::Printf(TEXT("Voice lgin result - %d"), Result.IsSuccess()));

	AShibPlayerState* ShibPS = Cast<AShibPlayerState>(PlayerState);
	if (!IsValid(ShibPS))
	{
		UE_LOG(LogShibPlayerController, Log, TEXT("%hs: Invalid Player State "), __FUNCTION__);
	}
	else
	{
		UE_LOG(LogShibPlayerController, Log, TEXT("%hs: Found UniqueNetID = %s"), __FUNCTION__,
		       *ShibPS->GetUniqueId().GetUniqueNetId()->ToString());
	}

	// Calling this server RPC to notify the server that the voice chat user has logged in, now it should be joined to a voice channel
	APlayerController* PC = Cast<APlayerController>(this);
	ServerNotifyLogin(PC);
}

void AShibPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}


void AShibPlayerController::Server_ToggleVoiceChat(bool bNewIsTalking)
{
	OnVoiceChatToggle.Broadcast(bNewIsTalking);
}

void AShibPlayerController::UnMuteRemotePlayer(const FUniqueNetIdPtr& RemotePlayerID)
{
	if (VoiceChatUser)
	{
		// Here we unmute this specific player for our player only
		VoiceChatUser->SetPlayerMuted(RemotePlayerID->ToString(), false);
	}
	else
	{
		// Doing this check because the VoiceChatUser will always be invalid on server, and we don't want it to show these logs
		if (!(GetNetMode() == NM_ListenServer || GetNetMode() == NM_DedicatedServer))
		{
			UE_LOG(LogShibPlayerController, Error, TEXT("%hs: VoiceChatUser Invalid"), __FUNCTION__)
		}
	}
}

void AShibPlayerController::MuteRemotePlayer(const FUniqueNetIdPtr& RemotePlayerID)
{
	if (VoiceChatUser)
	{
		// Here we mute this specific player for our player only
		VoiceChatUser->SetPlayerMuted(RemotePlayerID->ToString(), true);
	}
	else
	{
		// Doing this check because the VoiceChatUser will always be invalid on server, and we don't want it to show these logs
		if (!(GetNetMode() == NM_ListenServer || GetNetMode() == NM_DedicatedServer))
		{
			UE_LOG(LogShibPlayerController, Error, TEXT("%hs: VoiceChatUser Invalid"), __FUNCTION__)
		}
	}
}

#pragma endregion VoiceChat
