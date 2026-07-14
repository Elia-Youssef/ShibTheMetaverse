// Fill out your copyright notice in the Description page of Project Settings.

#include "ShibSessionsEOS.h"
#include "OnlineSubsystemUtils.h"
#include "ShibUserEOS.h"

DECLARE_LOG_CATEGORY_EXTERN(LogShibSessionSubsystem, Log, All);
DEFINE_LOG_CATEGORY(LogShibSessionSubsystem);

UShibSessionsEOS::UShibSessionsEOS() :
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UShibSessionsEOS::HandleCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UShibSessionsEOS::HandleFindSessionsComplete)),
	CancelFindSessionsCompleteDelegate(FOnCancelFindSessionsCompleteDelegate::CreateUObject(this, &UShibSessionsEOS::HandleCancelFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UShibSessionsEOS::HandleJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &UShibSessionsEOS::HandleDestroySessionComplete)),
	EndSessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &UShibSessionsEOS::HandleEndSessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &UShibSessionsEOS::HandleStartSessionComplete))
{
}

void UShibSessionsEOS::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	checkf(Subsystem != nullptr, TEXT("Unable to get Online Subsystem."));

	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!ensureMsgf(SessionInterface.IsValid(), TEXT("Unable to get the Session Interface"))) return;

	bCreateSessionOnDestroy = false;
}

void UShibSessionsEOS::Deinitialize()
{
	Super::Deinitialize();
}

void UShibSessionsEOS::CreateSession(FOnlineSessionSettingsBP SessionSettings)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!ensureMsgf(SessionInterface.IsValid(), TEXT("Unable to get the Session Interface"))) return;
	
	//Destroy existing session if it exists
	const auto ExistingSession = SessionInterface->GetNamedSession(ShibGameSession);
	if (ExistingSession != nullptr)
	{
		bCreateSessionOnDestroy = true;
		DestroySession();
		return;
	}

	//Register the delegate for when the creation complete and store its handle for later removal
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	//Create the session parameters. We could not use the MakeShareable and thus the new
	//but this way we can make SessionSettings a member of our class for future reuse.
	auto NativeSettings = new FOnlineSessionSettings(SessionSettings.ToNative());
	LastSessionSettings = MakeShareable(NativeSettings);

#if UE_BUILD_SHIPPING
	LastSessionSettings->BuildUniqueId = GetBuildUniqueId();
#else
	//Enforce a specific Build ID in not shipping so we can
	//easily test session creation
	LastSessionSettings->BuildUniqueId = 1;
#endif

	//Create the session
	const bool success = SessionInterface->CreateSession(0, ShibGameSession, *LastSessionSettings);
	if (!success)
	{
		//We failed to create the session simply remove the delegate for completion
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		CreateSessionCompleteDelegateHandle.Reset();
		OnCreateSessionComplete.Broadcast(false);
	}
}

void UShibSessionsEOS::FindSessions(UOnlineSessionSearch* SessionSearch)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	UShibUserEOS* UserSubsystem = GetGameInstance()->GetSubsystem<UShibUserEOS>();
	
	if (!ensureMsgf(SessionInterface && UserSubsystem, TEXT("Unable to get the Session or User Interface"))) return;

	if (!UserSubsystem->IsPlayerLoggedIn())
	{
		UE_LOG(LogShibSessionSubsystem, Error, TEXT("The player isn't logged in, impossible to start find session process."));
		return;
	}

	// Set session search
	// Get the native search object reference.
	LastSessionSearch = SessionSearch->ToNative();
	if (!LastSessionSearch.IsValid())
	{
		UE_LOG(LogShibSessionSubsystem, Error, TEXT("Invalid Session search settings, impossible to start find session process."));
		return;
	}
	
	// Add your search settings here...
	//To search for Presence session only, add the following search filter (For Steam and Epic ONLY):
	//SessionSearchRef->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	//To search for only non-listening sessions, add the following search filter:
	//SessionSearchRef->QuerySettings.Set(FName(TEXT("__EOS_bListening")), false, EOnlineComparisonOp::Equals);
	//To search for both listening and non-listening sessions, add the following search filter:
	//SessionSearchRef->QuerySettings.Set(FName(TEXT("__EOS_bListening")), FVariantData() /* Empty value */, EOnlineComparisonOp::Equals);
	// To include sessions which are full, add the following search filter:
	//SessionSearchRef->QuerySettings.SearchParams.Add(FName(TEXT("minslotsavailable")), FOnlineSessionSearchParam((int64)0L, EOnlineComparisonOp::GreaterThanEquals));
	
	// If you're including both listening and non-listening sessions (__EOS_bListening), then you must include at least one additional filter, or EOS will not return any search results.
	//SessionSearchRef->QuerySettings.Set(FName(TEXT("SettingValue")), FString(TEXT("SettingValue")), EOnlineComparisonOp::Equals);
	
	//Register the delegate for when the find session complete and store its handle for later removal
	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	const bool success = SessionInterface->FindSessions(*UserSubsystem->GetNetID().GetUniqueNetId(), LastSessionSearch.ToSharedRef());
	if (!success)
	{
		//We failed to find sessions simply remove the delegate for completion
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		FindSessionsCompleteDelegateHandle.Reset();

		OnFindSessionsComplete.Broadcast({}, false);
	}
}

bool UShibSessionsEOS::FindPreferredSession(const TArray<FOnlineSessionSearchResultBP> SessionsFound,
	const TArray<FOnlineSessionSearchResultBP> SessionsToIgnore, const int32 PreferredMaxPing, const int32 PreferredMinPlayerCount, FOnlineSessionSearchResultBP& PreferredSession)
{
	// Custom made function for filtering sessions
	// We need to keep in mind that the Session subsystem allows us to create and use custom filters directly within the system
	// So before adding more to this function, have a look at the subsystem first
	
	// filter out ignored sessions
	TArray<FOnlineSessionSearchResultBP> SessionResults;
	for (const auto& SessionResult : SessionsFound)
	{
		if (!SessionResult.Session.SessionInfo->IsValid()) continue;
		
		bool bIgnoreSession = false;
		
		for (const auto& IgnoredSession : SessionsToIgnore)
		{
			if (IgnoredSession.Session.SessionId == SessionResult.Session.SessionId)
			{
				bIgnoreSession = true;
				break;
			}
		}
		
		// don't use session if it's ignored
		if (!bIgnoreSession) SessionResults.Add(SessionResult);
	}
	
	// continue with an array of not ignored sessions
	
	if (SessionResults.IsEmpty())
    {
        return false; // No sessions available
    }

    // Find a "default" session with the best ping that isn't maxed out
    FOnlineSessionSearchResultBP DefaultSession;
    int32 BestPing = MAX_QUERY_PING;

    for (const FOnlineSessionSearchResultBP& Session : SessionResults)
    {
        const int32 SessionPing = Session.PingInMs;
        const int32 OpenPlayerSlots = Session.Session.NumOpenPublicConnections;

        if (OpenPlayerSlots > 0 && SessionPing < BestPing)
        {
            DefaultSession = Session;
            BestPing = SessionPing;
        }
    }
	
    if (BestPing == MAX_QUERY_PING)
    {
        return false; // No valid session with open slots
    }

    // Apply prioritization logic
	FOnlineSessionSearchResultBP PreferredSessionFound = DefaultSession;
    int32 PreferredPing = BestPing;
    int32 PreferredPlayerCount = 0;

    for (const FOnlineSessionSearchResultBP& Session : SessionResults)
    {
        const int32 SessionPing = Session.PingInMs;
        const int32 TotalPlayerSlots = Session.Session.SessionSettings.NumPublicConnections;
        const int32 OpenPlayerSlots = Session.Session.NumOpenPublicConnections;
        const int32 PlayerCount = TotalPlayerSlots - OpenPlayerSlots;

        if (OpenPlayerSlots <= 0)
        {
            continue; // Skip full sessions
        }
    	
        // Exclude sessions with more than 200 ping
        // Prioritize sessions with at least 20 players and find the lowest ping
    	// If sessions are lowly populated, find the highest populated one
        if (SessionPing <= PreferredMaxPing && ((PlayerCount >= PreferredMinPlayerCount && SessionPing < PreferredPing) ||
            PlayerCount > PreferredPlayerCount))
        {
            PreferredSessionFound = Session;
            PreferredPing = SessionPing;
            PreferredPlayerCount = PlayerCount;
        }
    }

	PreferredSession = PreferredSessionFound;
    return true;
}

void UShibSessionsEOS::CancelFindSessions()
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!ensureMsgf(SessionInterface.IsValid(), TEXT("Unable to get the Session Interface"))) return;

	CancelFindSessionsCompleteDelegateHandle = SessionInterface->AddOnCancelFindSessionsCompleteDelegate_Handle(CancelFindSessionsCompleteDelegate);

	if (const bool success = SessionInterface->CancelFindSessions(); !success)
	{
		SessionInterface->ClearOnCancelFindSessionsCompleteDelegate_Handle(CancelFindSessionsCompleteDelegateHandle);
		CancelFindSessionsCompleteDelegateHandle.Reset();
		
		OnCancelFindSessionsComplete.Broadcast(false);
	}
}
void UShibSessionsEOS::JoinSession(FOnlineSessionSearchResultBP SessionResult)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	UShibUserEOS* UserSubsystem = GetGameInstance()->GetSubsystem<UShibUserEOS>();
	
	if (!ensureMsgf(SessionInterface && UserSubsystem, TEXT("Unable to get the Session or User Interface"))) return;

	if (!UserSubsystem->IsPlayerLoggedIn())
	{
		UE_LOG(LogShibSessionSubsystem, Error, TEXT("The player isn't logged in, impossible to start join session process."));
		return;
	}
	
	//Register the delegate for when the join session complete and store its handle for later removal
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
	
	const bool success = SessionInterface->JoinSession(*UserSubsystem->GetNetID().GetUniqueNetId(), ShibGameSession, SessionResult.ToNative());
	if (!success)
	{
		//We failed to join session simply remove the delegate for completion
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		JoinSessionCompleteDelegateHandle.Reset();

		OnJoinSessionComplete.Broadcast(false, EShibJoinSessionResultTypeEOS::UnknownErrorEOS, "");
	}
}

void UShibSessionsEOS::StartSession()
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!ensureMsgf(SessionInterface.IsValid(), TEXT("Unable to get the Session Interface"))) return;

	StartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);

	if (!SessionInterface->StartSession(ShibGameSession))
	{
		//We failed to Start session simply remove the delegate for completion
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
		StartSessionCompleteDelegateHandle.Reset();
		OnStartSessionComplete.Broadcast(false);
	}
}

void UShibSessionsEOS::EndSession()
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!ensureMsgf(SessionInterface.IsValid(), TEXT("Unable to get the Session Interface"))) return;

	//Register the delegate for when the destroy session complete and store its handle for later removal
	EndSessionCompleteDelegateHandle = SessionInterface->AddOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegate);

	//End existing session if it exists
	const auto ExistingSession = SessionInterface->GetNamedSession(ShibGameSession);
	if (ExistingSession != nullptr)
	{
		SessionInterface->EndSession(ShibGameSession);
	}
	else //No session found
	{
		//Remove the end session completion delegate
		SessionInterface->ClearOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegateHandle);
		EndSessionCompleteDelegateHandle.Reset();

		OnEndSessionComplete.Broadcast(false);
	}
}

void UShibSessionsEOS::DestroySession()
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!ensureMsgf(SessionInterface.IsValid(), TEXT("Unable to get the Session Interface"))) return;

	//Register the delegate for when the destroy session complete and store its handle for later removal
	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	//Destroy existing session if it exists
	const auto ExistingSession = SessionInterface->GetNamedSession(ShibGameSession);
	if (ExistingSession != nullptr)
	{
		SessionInterface->DestroySession(ShibGameSession);
	}
	else //No session found
	{
		//Remove the destroy session completion delegate
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		DestroySessionCompleteDelegateHandle.Reset();

		OnDestroySessionComplete.Broadcast(false);
	}
}

bool UShibSessionsEOS::RegisterPlayer(APlayerController* InPlayerController)
{
	check(IsValid(InPlayerController));

	// This code handles logins for both the local player (listen server) and remote players (net connection).
	FUniqueNetIdRepl UniqueNetIdRepl;
	if (InPlayerController->IsLocalPlayerController())
	{
		ULocalPlayer* LocalPlayer = InPlayerController->GetLocalPlayer();
		if (IsValid(LocalPlayer))
		{
			UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
		}
		else
		{
			UNetConnection* RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
			check(IsValid(RemoteNetConnection));
			UniqueNetIdRepl = RemoteNetConnection->PlayerId;
		}
	}
	else
	{
		UNetConnection* RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
		check(IsValid(RemoteNetConnection));
		UniqueNetIdRepl = RemoteNetConnection->PlayerId;
	}

	// Get the unique player ID.
	TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
	check(UniqueNetId != nullptr);

	// Get the online session interface.
	const auto* Subsystem = Online::GetSubsystem(InPlayerController->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	
	// Register the player with the session name; this name should match the name you provided in CreateSession.
	bool RegisterResult = SessionInterface->RegisterPlayer(ShibGameSession, *UniqueNetId, false);
	UE_LOG(LogShibSessionSubsystem, Log, TEXT("Player %s registered: %s"), *UniqueNetId.Get()->ToString(), RegisterResult ? TEXT("true") : TEXT("false"));
	
	return RegisterResult;
}

bool UShibSessionsEOS::RegisterExistingPlayers()
{
	for (auto It = this->GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();

		// if (PlayerController->HasAuthority()) continue;

		// Register players, stop execution and return false if one of them has not successfully registered.
		if (!RegisterPlayer(PlayerController)) return false;
	}

	return true;
}

bool UShibSessionsEOS::UnregisterPlayer(APlayerController* InPlayerController)
{
	check(IsValid(InPlayerController));

	// This code handles logins for both the local player (listen server) and remote players (net connection).
	FUniqueNetIdRepl UniqueNetIdRepl;
	if (InPlayerController->IsLocalPlayerController())
	{
		ULocalPlayer* LocalPlayer = InPlayerController->GetLocalPlayer();
		if (IsValid(LocalPlayer))
		{
			UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
		}
		else
		{
			UNetConnection* RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
			check(IsValid(RemoteNetConnection));
			UniqueNetIdRepl = RemoteNetConnection->PlayerId;
		}
	}
	else
	{
		UNetConnection* RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
		check(IsValid(RemoteNetConnection));
		UniqueNetIdRepl = RemoteNetConnection->PlayerId;
	}

	// Get the unique player ID.
	TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
	check(UniqueNetId != nullptr);

	// Get the online session interface.
	const auto* Subsystem = Online::GetSubsystem(InPlayerController->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

	// Unregister the player with the session name; this name should match the name you provided in CreateSession.
	bool UnregisterResult = SessionInterface->UnregisterPlayer(ShibGameSession, *UniqueNetId);
	UE_LOG(LogShibSessionSubsystem, Log, TEXT("Player %s unregistered: %s"), *UniqueNetId.Get()->ToString(), UnregisterResult ? TEXT("true") : TEXT("false"));
	
	return UnregisterResult;
}

void UShibSessionsEOS::CleanUpSessions()
{
	// Get the online session interface. 
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

	//Destroy existing session locally if it exists
	const auto ExistingSession = SessionInterface->GetNamedSession(ShibGameSession);

	if (ExistingSession != nullptr)
	{
		DestroySession();
	}
}

bool UShibSessionsEOS::GetLastSessionSettings(FOnlineSessionSettingsBP& SessionSettings) const
{
	if (LastSessionSettings.IsValid())
	{
		SessionSettings = FOnlineSessionSettingsBP::FromNative(*LastSessionSettings.Get());
		return true;
	}
	return false;
}

bool UShibSessionsEOS::GetLastSessionSearch(UOnlineSessionSearch*& SessionSearch) const
{
	if (LastSessionSearch)
	{
		UOnlineSessionSearch* OutObj = NewObject<UOnlineSessionSearch>();
		OutObj->MaxSearchResults = LastSessionSearch->MaxSearchResults;
		OutObj->bIsLanQuery = LastSessionSearch->bIsLanQuery;
		OutObj->PingBucketSize = LastSessionSearch->PingBucketSize;
		OutObj->PlatformHash = LastSessionSearch->PlatformHash;
		OutObj->TimeoutInSeconds = LastSessionSearch->TimeoutInSeconds;
		OutObj->SearchParams.Reset();
		
		for (auto SearchParam : LastSessionSearch->QuerySettings.SearchParams)
		{
			FSessionSearchParamBP Param;

			Param.Op = (EOnlineComparisonOp_)SearchParam.Value.ComparisonOp;
			Param.Data.Type = (EOnlineKeyValuePairDataType_)SearchParam.Value.Data.GetType();
			
			switch (Param.Data.Type)
			{
			case EOnlineKeyValuePairDataType_::Int32:
				SearchParam.Value.Data.GetValue(Param.Data.AsInt);
				break;
			case EOnlineKeyValuePairDataType_::Int64:
				SearchParam.Value.Data.GetValue(Param.Data.AsInt64 );
				break;
			case EOnlineKeyValuePairDataType_::String:
				SearchParam.Value.Data.GetValue(Param.Data.AsString);
				break;
			case EOnlineKeyValuePairDataType_::Float:
				SearchParam.Value.Data.GetValue(Param.Data.AsFloat);
				break;
			case EOnlineKeyValuePairDataType_::Bool:
				SearchParam.Value.Data.GetValue(Param.Data.AsBool);
				break;
			case EOnlineKeyValuePairDataType_::Empty:
			case EOnlineKeyValuePairDataType_::Json_NotSupported:
			case EOnlineKeyValuePairDataType_::UInt32_NotSupported:
			case EOnlineKeyValuePairDataType_::UInt64_NotSupported:
			case EOnlineKeyValuePairDataType_::Double_NotSupported:
			case EOnlineKeyValuePairDataType_::Blob_NotSupported:
			default:
				break;
			}
			OutObj->SearchParams.Add(SearchParam.Key, Param);
		}
		SessionSearch = OutObj;
		return true;
	}
	return false;
}

void UShibSessionsEOS::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		CreateSessionCompleteDelegateHandle.Reset();
	}

	UE_LOG(LogShibSessionSubsystem, Log, TEXT("Session named %s created: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
	OnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UShibSessionsEOS::HandleFindSessionsComplete(bool bWasSuccessful)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		FindSessionsCompleteDelegateHandle.Reset();
	}

	if (!LastSessionSearch || LastSessionSearch->SearchResults.Num() <= 0)
	{
		UE_LOG(LogShibSessionSubsystem, Log, TEXT("Sessions found: false"));
		OnFindSessionsComplete.Broadcast({}, false);
		return;
	}

	UE_LOG(LogShibSessionSubsystem, Log, TEXT("Sessions found: true"));
	
	TArray<FOnlineSessionSearchResultBP> Results;
	for (auto Result : LastSessionSearch->SearchResults)
	{
		Results.Add(FOnlineSessionSearchResultBP::FromNative(Result));
	}
	
	OnFindSessionsComplete.Broadcast(Results, true);
}

void UShibSessionsEOS::HandleCancelFindSessionsComplete(bool bWasSuccessful)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCancelFindSessionsCompleteDelegate_Handle(CancelFindSessionsCompleteDelegateHandle);
		CancelFindSessionsCompleteDelegateHandle.Reset();
	}

	OnCancelFindSessionsComplete.Broadcast(true);
}

void UShibSessionsEOS::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	
	UE_LOG(LogShibSessionSubsystem, Log, TEXT("Session named %s joinned: %s"), *SessionName.ToString(), Result == EOnJoinSessionCompleteResult::Type::Success ? TEXT("true") : TEXT("false"));

	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		JoinSessionCompleteDelegateHandle.Reset();

		// Add network error handle if the game client fails to connect to the server
		NetworkFailureDelegateHandle = GEngine->OnNetworkFailure().AddUObject(this, &UShibSessionsEOS::HandleNetworkFailure);
	}
	else
	{
		OnJoinSessionComplete.Broadcast(Result == EOnJoinSessionCompleteResult::Type::Success, UnknownErrorEOS, "");
		return;
	}

	EShibJoinSessionResultTypeEOS ResultBP = EShibJoinSessionResultTypeEOS::UnknownErrorEOS;
	switch (Result)
	{
	case EOnJoinSessionCompleteResult::Success:
		ResultBP = ResultSuccessEOS;
		break;
	case EOnJoinSessionCompleteResult::SessionIsFull:
		ResultBP = SessionIsFullEOS;
		break;
	case EOnJoinSessionCompleteResult::SessionDoesNotExist:
		ResultBP = SessionDoesNotExistEOS;
		break;
	case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
		ResultBP = CouldNotRetrieveAddressEOS;
		break;
	case EOnJoinSessionCompleteResult::AlreadyInSession:
		ResultBP = AlreadyInSessionEOS;
		break;
	case EOnJoinSessionCompleteResult::UnknownError:
		ResultBP = UnknownErrorEOS;
		break;
	}

	//Get the session address to make a client travel
	FString Address;
	SessionInterface->GetResolvedConnectString(ShibGameSession, Address);
	
	//Fire our own delegate
	OnJoinSessionComplete.Broadcast(Result == EOnJoinSessionCompleteResult::Type::Success, ResultBP, Address);
}

void UShibSessionsEOS::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		DestroySessionCompleteDelegateHandle.Reset();

		// Unregister network failure handler when we destroy the current session
		if (NetworkFailureDelegateHandle.IsValid())
		{
			GEngine->OnNetworkFailure().Remove(NetworkFailureDelegateHandle);
			NetworkFailureDelegateHandle.Reset();
		}
	}

	UE_LOG(LogShibSessionSubsystem, Log, TEXT("Session named %s destroyed: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
	OnDestroySessionComplete.Broadcast(bWasSuccessful);

	// Check if we need to create another session right after it got destroyed
	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false; // Make sure to set this back to false to not get stuck in a loop
		CreateSession(FOnlineSessionSettingsBP::FromNative(*LastSessionSettings.Get()));
	}
}

void UShibSessionsEOS::HandleEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegateHandle);
		EndSessionCompleteDelegateHandle.Reset();
	}

	UE_LOG(LogShibSessionSubsystem, Log, TEXT("Session named %s ended: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
	OnEndSessionComplete.Broadcast(bWasSuccessful);
}

void UShibSessionsEOS::HandleStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
		StartSessionCompleteDelegateHandle.Reset();
	}

	UE_LOG(LogShibSessionSubsystem, Log, TEXT("Session named %s started: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
	OnStartSessionComplete.Broadcast(bWasSuccessful);
}

void UShibSessionsEOS::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver,
	ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	const auto* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		// Optional: Notify players or handle reconnection logic
	}

	// Unregister network failure handler
	GEngine->OnNetworkFailure().Remove(NetworkFailureDelegateHandle);
	NetworkFailureDelegateHandle.Reset();
}
