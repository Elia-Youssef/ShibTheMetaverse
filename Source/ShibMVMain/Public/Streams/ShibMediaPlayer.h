// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "MediaPlayer.h"
#include "ShibMediaPlayer.generated.h"

/**
 * 
 */
UCLASS()
class SHIBMVMAIN_API UShibMediaPlayer : public UMediaPlayer
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Initialize(UWorld* WorldContext);

private:
	void StartTick();
	void StopTick();
	
	UFUNCTION()
	void PlaybackTick();
	
	UFUNCTION()
	void OnMediaOpened_Implementation(FString OpenedUrl);

	UFUNCTION()
	void OnMediaClosed_Implementation();

	UFUNCTION()
	void OnPlaybackResumed_Implementation();

	UFUNCTION()
	void OnPlaybackSuspended_Implementation();

	UFUNCTION()
	void OnEndReached_Implementation();

	UFUNCTION()
	void OnSeekCompleted_Implementation();
	
	UFUNCTION()
	void OnFocusChanged(bool bFocus);

	bool bIsWindowFocused = true;
	
	bool bRecentReset = true;
	int32 TicksToIgnoreAfterReset = 3;
	int32 TicksAfterReset = 0;
	
	int32 VideoStuckThreshold = 3;
	int32 VideoStuckTicks = 0;
	
	FTimespan SeekTo = FTimespan::Zero();
	FTimespan VideoDuration = FTimespan::Zero();
	FTimespan TimeLeft = FTimespan::Zero();

	UPROPERTY()
	UWorld* World;
	FTimerHandle PlaybackTimer;
};
