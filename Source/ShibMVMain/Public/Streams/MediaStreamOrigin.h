// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibAPIsTypes.h"
#include "GameFramework/Actor.h"
#include "MediaStreamOrigin.generated.h"

class UMetaverseApisSubsystem;
class UShibMediaStreamer;
class UMediaPlayer;
class UShibAPIsSubsystem;
class UShibGameInstance;

UCLASS()
class SHIBMVMAIN_API AMediaStreamOrigin : public AActor
{
	GENERATED_BODY()

public:
	AMediaStreamOrigin();

	/**
	 * Adds StreamerId to the list of Ids to fetch data for
	 */
	UShibMediaStreamer* AddComponent(FString StreamerId);

	/**
	 * Time to wait for StreamerIds to be added
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stream Config|Server")
	int32 InitialDelay = 10;

	/**
	 * Rate to sync the media array with the server (seconds) 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stream Config|Server")
	int32 SyncRate = 60;

	/**
	 * How many images should we keep in memory?
	 * Higher -> high memory usage, low bandwidth usage
	 * Lower -> low memory usage, high bandwidth usage
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stream Config|Client")
	int32 MaxCacheBufferSize = 3;

protected:
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void Server_GetMedia();

	UFUNCTION(Client, Reliable)
	void Client_GetMedia(const TArray<FMedia>& Media);

	/**
	 * Starts the syncing timer after an initial delay
	 */
	void InitializeServerSync();

	/**
	 * Calls the backend to get the media array
	 */
	UFUNCTION()
	void GetMediaFromServer();
	UFUNCTION()
	void OnGetMediaFromServer(const TArray<FMedia>& Media, bool bSuccessful);

	/**
	 * All Ids in this level to get data for and their video players
	 */
	// TMap<int32, UMediaPlayer*> MediaComponentsInLevel;

	UPROPERTY()
	TMap<FString, UShibMediaStreamer*> Streamers;

	/**
	 * Array of Media to replicate
	 */
	UPROPERTY(ReplicatedUsing=OnRep_MediaArray)
	TArray<FMedia> MediaArray;
	UFUNCTION()
	void OnRep_MediaArray();

private:
	UPROPERTY()
	UShibGameInstance* ShibGI = nullptr;

	UPROPERTY()
	UMetaverseApisSubsystem* MvAPIs = nullptr;

	FTimerHandle InitialDelayHandle;
	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;
};
