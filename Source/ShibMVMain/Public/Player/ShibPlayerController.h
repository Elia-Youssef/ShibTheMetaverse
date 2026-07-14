// Copyright Shib LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibPlayerState.h"
#include "Inventory/ShibInventory.h"
#include "Player/ShibBasePlayerController.h"
#include "ShibPlayerController.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogShibPlayerController, Log, All);

class UShibChatComponent;
class UShibInventory;
struct FVoiceChatResult;
class IVoiceChatUser;
class IVoiceChat;
class UInputMappingContext;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FShibOnPreTeleportToLocation, FName, PodTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FShibOnPostTeleportToLocation, bool, bTeleportSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FShibVoiceChatToggle, bool, bMute);

UCLASS()
class SHIBMVMAIN_API AShibPlayerController : public AShibBasePlayerController
{
	GENERATED_BODY()
	
public:
	AShibPlayerController();

	// We override this function to receive sound from the pawn location instead of the player controller location (Camera view)
	// Better when we're in a third person view
	virtual void GetAudioListenerPosition(FVector& OutLocation, FVector& OutFrontDir, FVector& OutRightDir) const override;

	// Class reference for the main player HUD
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=ShibController)
	TSubclassOf<UUserWidget> PlayerHudRef;

	//Widget reference of the main player HUD
	UPROPERTY(BlueprintReadWrite, Category=ShibController)
	TObjectPtr<UUserWidget> PlayerHud;
	
	// ====== CHAT ======
	
	TObjectPtr<IVoiceChat> VoiceChat;
	TObjectPtr<IVoiceChatUser> VoiceChatUser;

	UPROPERTY(BlueprintReadWrite, Category="ShibController | Player Chat")
	UShibChatComponent* ShibChatComponent;

	void UnMuteRemotePlayer(const FUniqueNetIdPtr& RemotePlayerID);

	void MuteRemotePlayer(const FUniqueNetIdPtr& RemotePlayerID);
	
	void OnNewPlayerLoggedIn(FUniqueNetIdPtr RemotePlayerID);

	// demo
	// UFUNCTION(Server, Reliable)
	void ServerNotifyLogin(APlayerController * NewPlayer);

	// demo
	// UFUNCTION(Server, Reliable)
	void ServerNotifyChannelJoin(AShibPlayerController * NewPlayer);

	// demo
	// UFUNCTION(Client, Reliable)
	void JoinVoiceChannel(const FString& InChannelName, const FString& InChannelCredentials);

	// demo
	// UFUNCTION(BlueprintCallable, Client, Reliable)
	void LeaveAllChannels();

	// ====== TELEPORTATION ======

	/**This is used to prevent pawn spawning before the teleportation TAG is received by the server.*/
	bool bTagHasBeenConfirmed = false;

	UPROPERTY(EditAnywhere,Category="ShibController | Traveling")
	FVector RelativeLocationWhenTeleporting = FVector(0.f, 0.f, 200.f);

	/**
	 * Delegate called right before we teleport locally to a different location within the same level
	 * Note: Should be called on client only
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="ShibController | Traveling")
	FShibOnPreTeleportToLocation OnPreTeleportToLocation;

	/**
	 * Delegate called after we teleported locally to a different location within the same level
	 * Note: Should be called on client only
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="ShibController | Traveling")
	FShibOnPostTeleportToLocation OnPostTeleportToLocation;

	/**
	 * Travel to a different pod location in a different level, it can be within the current level, or elsewhere in another level.
	 * The function takes care of finding if the location is within the current level or not, and/or start a client
	 * travel to another session/map to the desired location.
	 * Note: This function isn't restricted to client only, but it doesn't make sense to use this function on the server.
	 * @param LevelKey The level ID key from the data table that defines a specific level (map)
	 * @param PodTag The tag ID of the pod object in the level which indicates our spawn location
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="ShibController | Traveling")
	void TravelToLocation(const FName LevelKey, const FName PodTag);

	/**
	 * Travel to a different pod location within the same level.
	 * @param PodTag The tag ID of the pod object in the level which indicates our spawn location
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="ShibController | Traveling")
	void TravelToLocalLocation(const FName PodTag);

	// demo
	// UFUNCTION(Server, Reliable, BlueprintCallable, Category="ShibController | Traveling")
	UFUNCTION(BlueprintCallable, Category="ShibController | Traveling")
	void Server_TeleportToLocation(const FName& PodTag);

	/**
	 * Client function to allow the client to configure things after being teleported to the new location.
	 * This is a client function since the teleport function is executed on the server.
	 * The server will call this function after teleporting the player.
	 */
	// Commented out for demo
	// UFUNCTION(Client, Reliable)
	void Client_OnPostTeleportToLocationHandle(bool bTeleportSuccess);

	// Commented out for demo
	// UFUNCTION(Client, Reliable)
	void Client_GetTeleportLocation();

	// ==================

	virtual void BeginReplication() override;
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void OnPossess(APawn* InPawn) override;

	virtual void ClientTravelToSession(const FString Address) override;
	
	/**
	 * Create in game HUD using the HUD widget class reference
	 * The variable PlayerHudRef must be valid for it to work and create the widget on screen.
	 * We can retrieve the HUD object after its creation using the PlayerHud variable.
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category=ShibController)
	void CreateGameHud();

public:
	/**
	 * For quickly enable/disable default game inputs by adding or removing the default mapping from the character.
	 * A valid default mapping reference in the character is necessary. 
	 * @param bEnabled Weather or not we should add or remove the Mapping contexts
	 * @param MappingContexts Mapping contexts to add or remove
	 */
	UFUNCTION(BlueprintCallable, Category="ShibController | Inputs")
	void ToggleMappingContexts(bool bEnabled, TArray<UInputMappingContext*> MappingContexts);

protected:
#pragma region Inventory
public:
	UFUNCTION(BlueprintPure)
	UShibInventory* GetShibInventory() const;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UShibInventory* ShibInventoryComponent = nullptr;

	void InitializeInventory(int32 UserId);
	
	UFUNCTION(BlueprintPure)
	TArray<FShibItem> GetShibItems() const;

	UFUNCTION()
	void HandleInventoryUpdate();

	UFUNCTION(BlueprintImplementableEvent)
	void OnInventoryUpdated();
	
#pragma endregion Inventory
	
#pragma region VoiceChat
	// ====== CHAT ======

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="ShibController | Player Chat")
	FShibVoiceChatToggle OnVoiceChatToggle;


protected:
	UFUNCTION(BlueprintCallable, Category="ShibController | Player Chat")
	void InitVoiceChat();

	UFUNCTION(BlueprintCallable, Category="ShibController | Player Chat")
	void SetVoiceInputVolume(float Volume);
	
	UFUNCTION(BlueprintCallable, Category="ShibController | Player Chat")
	void SetVoiceOutputVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category="ShibController | Player Chat")
	void MuteSelf(bool Mute);
	
	void OnVoiceLoginComplete(const FString& PlayerName, const FVoiceChatResult& Result);

	virtual void OnRep_PlayerState() override;
	
	/**Possibly redundant*/
	// Commented out for demo
	// UFUNCTION(Server, Reliable)
	void Server_ToggleVoiceChat(bool bNewIsTalking);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ShibController | Player Chat")
	float InputVolume = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ShibController | Player Chat")
	float OutputVolume = 100.f;

	// ==================
	#pragma endregion VoiceChat
};
