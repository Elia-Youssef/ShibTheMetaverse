#pragma once

#include "CoreMinimal.h"
#include "RedpointMatchmaking/MatchmakingEngine.h"

#include "Subsystems/GameInstanceSubsystem.h"
#include "ShibMatchmakingEOS.generated.h"

USTRUCT(BlueprintType)
struct SHIBMULTIPLAYER_API FShibMatchmakerHostConfiguration
{
	GENERATED_BODY()
	
public:
    FShibMatchmakerHostConfiguration()
        : QueueName()
        , TeamCapacities()
        , SkillStatPrefix()
        , Map(){};
        //, SessionFilters(){};

    /**
     * The name of the matchmaking queue to queue into.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    FString QueueName = TEXT("Default");
	
	/**
	 * A string like '3v3v3' (3 teams of 3) or '4x20' (20 teams of 4).
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
	FString TeamCapacities;

    /**
     * If true, finish matchmaking with a partially filled match if no further candidates could be found within the
     * specified timeout.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    bool bAllowPartiallyFilledMatches = true;

    /**
     * If partial matches is enabled, this is the timeout in seconds after which, if we still can't find any other
     * candidates to match with, a partially filled match will be returned. Note that this is *not* a global timeout for
     * the entire matchmaking process. It is just the time since the last player was added to the match.
     *
     * The matchmaker has a minimum of 15 seconds for this value; if you set this to any lower than 15 seconds, the
     * matchmaker will use 15 seconds instead.
     */
    UPROPERTY(
        BlueprintReadWrite,
        EditAnywhere,
        Category = "Matchmaking",
        meta = (EditCondition = "bAllowPartiallyFilledMatches", ClampMin = "15"))
    int32 NoCandidatesTimeout = 60;

    /**
     * The time in seconds to add to the timeout for each currently empty slot in the match. If this is non-zero,
     * matchmaking will wait longer to return a partial match, the more empty the match currently is.
     */
    UPROPERTY(
        BlueprintReadWrite,
        EditAnywhere,
        Category = "Matchmaking",
        meta = (EditCondition = "bAllowPartiallyFilledMatches", ClampMin = "0"))
    int32 NoCandidatesTimeoutPerEmptySlot = 5;

    /**
     * If true, matchmaking will prioritize having balanced teams throughout the matchmaking process, rather than
     * prioritizing having completely full teams. Enabling this option means a partially filled match is more likely to
     * have a balanced number of players, at the cost of having a higher chance that a match will be partially filled if
     * there are not a lot of solo players entering the matchmaking queue.
     */
    UPROPERTY(
        BlueprintReadWrite,
        EditAnywhere,
        Category = "Matchmaking",
        meta = (EditCondition = "bAllowPartiallyFilledMatches"))
    bool bPrioritizeBalance = false;

    /**
     * If set to something other than empty string, enables skill-based matchmaking. The <prefix>_mu and <prefix>_sigma
     * stats will be looked up in the online stats interface to perform skill-based matchmaking, so you must ensure
     * you've created the appropriate stats in the backend.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    FString SkillStatPrefix;

    /**
     * If true, the host will search for a dedicated server to play on once the match is ready, instead of hosting the
     * game themselves on a listen server.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Matchmaking")
    bool bUseDedicatedServers = false;

    /**
     * The map to start when matchmaking is complete (listen servers only).
     */
    UPROPERTY(
        BlueprintReadWrite,
        EditAnywhere,
        Category = "Matchmaking (Listen Servers)",
        meta = (EditCondition = "!bUseDedicatedServers"))
    TSoftObjectPtr<UWorld> Map;

    /**
     * The filters to apply when searching for dedicated servers.
     * NOTE: NOT IN USE FOR NOW
     */
	/*
    UPROPERTY(
        BlueprintReadWrite,
        EditAnywhere,
        Category = "Matchmaking (Dedicated Servers)",
        meta = (EditCondition = "bUseDedicatedServers"))
    TArray<FMatchmakerHostConfigurationAttributeFilter> SessionFilters;
	*/
	
    /**
     * The port that the dedicated server matchmaking beacon listens on.
     */
    UPROPERTY(
        BlueprintReadWrite,
        EditAnywhere,
        Category = "Matchmaking (Dedicated Servers)",
        meta = (EditCondition = "bUseDedicatedServers"))
    FName BeaconPort = FName(TEXT("9990"));
};

// We just make blueprint visible events so we can pass the
// information up to our widgets
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnGameMatchmakingProgress, FName, StepName, FText, Status, FText, Detail, float, Progress, FDateTime, ETA);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameMatchmakingComplete, FString, MatchResults);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameMatchmakingCancelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameMatchmakingError);

/**
 * 
 */
UCLASS()
class SHIBMULTIPLAYER_API UShibMatchmakingEOS : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	public:

	explicit UShibMatchmakingEOS();
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable)
	FOnGameMatchmakingProgress OnProgress;

	UPROPERTY(BlueprintAssignable)
	FOnGameMatchmakingComplete OnComplete;

	UPROPERTY(BlueprintAssignable)
	FOnGameMatchmakingCancelled OnCancelled;

	UPROPERTY(BlueprintAssignable)
	FOnGameMatchmakingError OnError;

	UFUNCTION(BlueprintCallable)
	void StartMatchmaking(FShibMatchmakerHostConfiguration MatchMakingHostConfig);

	UFUNCTION(BlueprintCallable)
	void CancelMatchmaking();

private:
	FMatchmakingEngineRequestHandle Handle;

	void OnHandleProgress(
		const FString &TaskId,
		const struct FMatchmakingEngineRequest &Request,
		FMatchmakingEngineProgressInfo ProgressInfo);
	void OnHandleResultsReady(
		const FString &TaskId,
		const struct FMatchmakingEngineRequest &Request,
		FMatchmakingEngineResponse Response);
	void OnHandleComplete(
		const FString &TaskId,
		const struct FMatchmakingEngineRequest &Request,
		FMatchmakingEngineResponse Response);
	void OnHandleCancelled(const FString &TaskId, const struct FMatchmakingEngineRequest &Request);
	void OnHandleError(
		const FString &TaskId,
		const struct FMatchmakingEngineRequest &Request,
		const FOnlineError &Error);
	FSearchParams OnHandleGetDedicatedServerSearchParams(
		const FString &TaskId,
		const struct FMatchmakingEngineRequest &Request,
		FMatchmakingEngineResponse Response,
		int32 SearchIteration);
};
