// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Types/OSBSessionTypes.h"
#include "ShibBasePlayerController.generated.h"

enum EShibJoinSessionResultTypeEOS : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FShibInternalOnSearchOrJoinSessionError);
	
/**
 * 
 */
UCLASS()
class SHIBMVMAIN_API AShibBasePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AShibBasePlayerController();

	// Widget Class to display when searching for a session
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Session)
	TSubclassOf<UUserWidget> SessionsSearchWidgetRef;

	/**
	 * Notify client they were kicked from the server.
	 * Base class kick function.
	 * Both kick function call the same internal function.
	 * @param KickReason The reason why the player got kicked out from the session
	 */
	void ClientWasKicked(const FText& KickReason);

	/**
	 * Custom kick function used in our system.
	 * Notify player that he was kicked out of the session.
	 * @param KickReason The reason why the player got kicked out from the session
	 */
	UFUNCTION(Client, Reliable)
	void Client_WasKickedFromSession(const FText& KickReason);
	
	virtual void OnNetCleanup(UNetConnection* Connection) override;

	/**
	 * Start looking for available sessions.
	 * This will also join the first session found.
	 * @param SearchSettings Search settings for the search session process
	 */
	UFUNCTION(BlueprintCallable, Category=Session)
	void FindAndJoinSession(UOnlineSessionSearch* SearchSettings);

	/**
	 * Cancel the find session process
	 */
	UFUNCTION(BlueprintCallable, Category=Session)
	void CancelFindSessions();

	/**
	 * Delegate called when something wrong happens internally within this class during the find and join session process
	 * Any actor launching the process from this player controller should bind itself
	 * to this event as well as all the other ones from the session subsystem.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category=Session)
	FShibInternalOnSearchOrJoinSessionError InternalOnFindOrJoinSessionError;
	
protected:
	virtual void OnRep_Pawn() override;

	/**
	 * Session Client travel. Expose the ClientTravel function to blueprint.
	 * @param Address The address of the session to travel to
	 */
	UFUNCTION(BlueprintCallable)
	virtual void ClientTravelToSession(const FString Address);

	/**
	 * Give a chance to the blueprint class to do some stuff after the player got kicked out from the session.
	 * @param KickReason The reason why the player got kicked out from the session
	 */
	UFUNCTION(BlueprintImplementableEvent, Category=Session)
	void InternalClientWasKickedFromSession(const FText& KickReason);

	/**
	 * Give a chance to the blueprint class to do some stuff before starting the search and join session process.
	 * @param SearchSettings Search settings for the search session process
	 */
	UFUNCTION(BlueprintImplementableEvent, Category=Session)
	void InternalFindAndJoinSession(UOnlineSessionSearch* SearchSettings);
	
	/**
	 * Handle function after asking for the available session to the EOS Session Subsystem.
	 * @param SessionResults Array of sessions found 
	 * @param bWasSuccessful If we actually found one or more available sessions
	 */
	UFUNCTION()
	void OnSessionFoundHandle(const TArray<FOnlineSessionSearchResultBP>& SessionResults, bool bWasSuccessful);
	/**
	 * Give a chance to the blueprint class to do some stuff after we found available sessions.
	 * @param SessionResults Array of sessions found
	 * @param bWasSuccessful If we actually found one or more available sessions
	 */
	UFUNCTION(BlueprintImplementableEvent, Category=Session)
	void InternalOnSessionFound(const TArray<FOnlineSessionSearchResultBP>& SessionResults, bool bWasSuccessful);
	
	/**
	 * Handle function after asking to join a session to the EOS Session Subsystem.
	 * @param bWasSuccessful If the join process was successful
	 * @param Type Result type, allows you to quickly know the status of the request with when it ends
	 * @param Address The address of the session you just joined, used to make a client travel to the session
	 */
	UFUNCTION()
	void OnSessionJoinedHandle(bool bWasSuccessful, const EShibJoinSessionResultTypeEOS Type, const FString& Address);
	/**
	 * Give a chance to the blueprint class to do some stuff after joining a session.
	 * @param bWasSuccessful If the join process was successful
	 * @param Type Result type, allows you to quickly know the status of the request with when it ends
	 * @param Address The address of the session you just joined, used to make a client travel to the session
	 */
	UFUNCTION(BlueprintImplementableEvent, Category=Session)
	void InternalOnSessionJoined(bool bWasSuccessful, EShibJoinSessionResultTypeEOS Type, const FString& Address);
	
	/**
	 * Handle function after cancelling the find sessions process
	 * @param bWasSuccessful If the cancellation was successful
	 */
	UFUNCTION()
	void OnCancelFindSessionsHandle(bool bWasSuccessful);
	/**
	 * Give a chance to the blueprint class to do some stuff when cancelling the find and join session process
	 * This happens right before actually cancelling the process in the subsystem
	 */
	UFUNCTION(BlueprintImplementableEvent, Category=Session)
	void InternalOnCancelFindSessions();
};
