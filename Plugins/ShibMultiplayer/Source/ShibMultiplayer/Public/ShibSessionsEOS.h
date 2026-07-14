#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "FindSessionsCallbackProxy.h"
#include "Engine/GameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Templates/SharedPointer.h"
#include "GameFramework/OnlineReplStructs.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Engine/NetConnection.h"
#include "Types/OSBSessionTypes.h"
#include "ShibSessionsEOS.generated.h"

struct FBlueprintSessionResult;
enum EShibJoinSessionResultTypeEOS : uint8;

/**
 * Struct that holds player data for the leaderboard
 */
USTRUCT(BlueprintType)
struct FLeaderboardRowEOS
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly,Category="Shib Session Subsystem")
	float Score = 0.f;

	UPROPERTY(BlueprintReadOnly,Category="Shib Session Subsystem")
	FString PlayerName = FString();
};

/**
 * Struct that holds game round data
 */
USTRUCT(BlueprintType)
struct FRoundEOS
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly,Category="Shib Session Subsystem")
	int32 Index = 0;

	UPROPERTY(BlueprintReadOnly,Category="Shib Session Subsystem")
	TArray<FLeaderboardRowEOS> Leaderboard = TArray<FLeaderboardRowEOS>();
};

UENUM(BlueprintType)
enum EShibJoinSessionResultTypeEOS : uint8
{
	/** The join worked as expected */
	ResultSuccessEOS  UMETA(DisplayName = "ResultSuccess"),
	/** There are no open slots to join */
	SessionIsFullEOS UMETA(DisplayName = "Full"),
	/** The session couldn't be found on the service */
	SessionDoesNotExistEOS UMETA(DisplayName = "DoesNotExist"),
	/** There was an error getting the session server's address */
	CouldNotRetrieveAddressEOS UMETA(DisplayName = "CouldNotRetrieveAddress"),
	/** The user attempting to join is already a member of the session */
	AlreadyInSessionEOS UMETA(DisplayName = "AlreadyIn"),
	/** An error not covered above occurred */
	UnknownErrorEOS UMETA(DisplayName = "UnknownError")
};

UENUM(BlueprintType)
enum EShibSearchSessionTypeEOS : uint8
{
	/* Search for public and private sessions */
	AllEOS UMETA(DisplayName = "All"),
	/* Search for private sessions */
	PrivateEOS UMETA(DisplayName = "Private"),
	/* Search for public sessions */
	PublicEOS UMETA(DisplayName = "Public"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FShibOnCreateSessionCompleteEOS, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FShibOnFindSessionCompleteEOS, const TArray<FOnlineSessionSearchResultBP>&, SessionResults, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FShibOnJoinSessionCompleteEOS, bool, bWasSuccessful, EShibJoinSessionResultTypeEOS, Type, const FString&, Address);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FShibOnCancelFindSessionsCompleteEOS, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FShibOnDestroySessionCompleteEOS, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FShibOnEndSessionCompleteEOS, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FShibOnStartSessionCompleteEOS, bool, bWasSuccessful);	

#define ShibGameSession FName("GameSession")

UCLASS()
class SHIBMULTIPLAYER_API UShibSessionsEOS : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	public:

	UShibSessionsEOS();
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	// Public interface to start session creation
	UFUNCTION(BlueprintCallable, Category = "Shib Subsystems|Sessions")
	void CreateSession(FOnlineSessionSettingsBP SessionSettings);

	// Function to initiate a session search
	UFUNCTION(BlueprintCallable, Category = "Shib Subsystems|Sessions")
	void FindSessions(UOnlineSessionSearch* SessionSearch);

	// Function to filter sessions found to find the best one. This looks at all the sessions and excludes the ones from the sessions to ignore list, compare the ping of each session and excludes the ones with high ping.
	UFUNCTION(BlueprintCallable, Category = "Shib Subsystems|Sessions", meta=(ReturnDisplayName = "Success"))
	bool FindPreferredSession(const TArray<FOnlineSessionSearchResultBP> SessionsFound, const TArray<FOnlineSessionSearchResultBP> SessionsToIgnore,
		const int32 PreferredMaxPing, const int32 PreferredMinPlayerCount, FOnlineSessionSearchResultBP& PreferredSession);
	
	// Function to join a session
	UFUNCTION(BlueprintCallable, Category = "Shib Subsystems|Sessions")
	void JoinSession(FOnlineSessionSearchResultBP SessionResult);

	// Function to start the session
	UFUNCTION(BlueprintCallable, Category = "Shib Subsystems|Sessions")
	void StartSession();

	UFUNCTION(BlueprintCallable, Category = "Shib Subsystems|Sessions")
	void CancelFindSessions();

	// Function to end the session
	UFUNCTION(BlueprintCallable, Category = "Shib Subsystems|Sessions")
	void EndSession();
	
	// Function to destroy the session
	UFUNCTION(BlueprintCallable, Category = "Shib Subsystems|Sessions")
	void DestroySession();

	// Function to register incomming player to the session
	UFUNCTION(BlueprintCallable, Category = "Shib Subsystems|Sessions")
	bool RegisterPlayer(APlayerController* InPlayerController);

	// Function to register existing players currently in the session
	UFUNCTION(BlueprintCallable, Category = "Shib Subsystems|Sessions")
	bool RegisterExistingPlayers();

	// Function to unregister player from the session
	UFUNCTION(BlueprintCallable, Category = "Shib Subsystems|Sessions")
	bool UnregisterPlayer(APlayerController* InPlayerController);

	// Function to Check Session State
	UFUNCTION(BlueprintCallable, Category = "Shib Subsystems|Sessions")
	void CleanUpSessions();
	
	// Get the last session settings
	UFUNCTION(BlueprintPure, Category = "Shib Subsystems|Sessions", meta=(ReturnDisplayName="SettingsFound"))
	bool GetLastSessionSettings(FOnlineSessionSettingsBP& SessionSettings) const;

	// Get the last session settings
	UFUNCTION(BlueprintPure, Category = "Shib Subsystems|Sessions", meta=(ReturnDisplayName="SessionSearchFound"))
	bool GetLastSessionSearch(UOnlineSessionSearch*& SessionSearch) const;
	
	//
	// Public delegates for class to bind callbacks
	//
	UPROPERTY(BlueprintAssignable, Category = "Shib Subsystems|Sessions|Callback")
	FShibOnCreateSessionCompleteEOS OnCreateSessionComplete;

	UPROPERTY(BlueprintAssignable, Category = "Shib Subsystems|Sessions|Callback")
	FShibOnFindSessionCompleteEOS OnFindSessionsComplete;

	UPROPERTY(BlueprintAssignable, Category = "Shib Subsystems|Sessions|Callback")
	FShibOnJoinSessionCompleteEOS OnJoinSessionComplete;

	UPROPERTY(BlueprintAssignable, Category = "Shib Subsystems|Sessions|Callback")
	FShibOnDestroySessionCompleteEOS OnDestroySessionComplete;

	UPROPERTY(BlueprintAssignable, Category = "Shib Subsystems|Sessions|Callback")
	FShibOnCancelFindSessionsCompleteEOS OnCancelFindSessionsComplete;

	UPROPERTY(BlueprintAssignable, Category = "Shib Subsystems|Sessions|Callback")
	FShibOnEndSessionCompleteEOS OnEndSessionComplete;

	UPROPERTY(BlueprintAssignable, Category = "Shib Subsystems|Sessions|Callback")
	FShibOnStartSessionCompleteEOS OnStartSessionComplete;

protected:
	//
	// Internal callbacks for the delegates.
	//
	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleFindSessionsComplete(bool bWasSuccessful);
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void HandleCancelFindSessionsComplete(bool bWasSuccessful);
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleEndSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleStartSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

private:
	
	bool bCreateSessionOnDestroy{ false };

	// Cached settings of the last session created
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	// Cached settings of the last session search
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
	
	//
	// Delegate Handles for the OnlineSubsystem session interfaces
	//
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;

	FOnCancelFindSessionsCompleteDelegate CancelFindSessionsCompleteDelegate;
	FDelegateHandle CancelFindSessionsCompleteDelegateHandle;
	
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	
	FOnEndSessionCompleteDelegate EndSessionCompleteDelegate;
	FDelegateHandle EndSessionCompleteDelegateHandle;
	
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;
	
	FDelegateHandle NetworkFailureDelegateHandle;
	
};
