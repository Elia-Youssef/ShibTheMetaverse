// Copyright Shiba Inu Games LLC.


#include "Streams/ShibMediaPlayer.h"


void UShibMediaPlayer::Initialize(UWorld* WorldContext)
{
	if (!WorldContext) return;

	PlayOnOpen = false;
	World = WorldContext;

	OnMediaOpened.AddUniqueDynamic(this, &ThisClass::OnMediaOpened_Implementation);
	OnMediaClosed.AddUniqueDynamic(this, &ThisClass::OnMediaClosed_Implementation);
	OnPlaybackResumed.AddUniqueDynamic(this, &ThisClass::OnPlaybackResumed_Implementation);
	OnPlaybackSuspended.AddUniqueDynamic(this, &ThisClass::OnPlaybackSuspended_Implementation);
	OnEndReached.AddUniqueDynamic(this, &ThisClass::OnEndReached_Implementation);
	OnSeekCompleted.AddUniqueDynamic(this, &ThisClass::OnSeekCompleted_Implementation);

	FSlateApplication::Get().OnApplicationActivationStateChanged().AddUObject(this, &ThisClass::OnFocusChanged);
}

void UShibMediaPlayer::StartTick()
{
	StopTick();
	// UE_LOG(LogTemp, Log, TEXT("%hs"), __FUNCTION__);
	
	FTimerDelegate PlaybackDelegate;
	PlaybackDelegate.BindUObject(this, &ThisClass::PlaybackTick);
	World->GetTimerManager().SetTimer(PlaybackTimer, PlaybackDelegate, .25f, true);
}

void UShibMediaPlayer::StopTick()
{
	// UE_LOG(LogTemp, Log, TEXT("%hs"), __FUNCTION__);
	
	World->GetTimerManager().ClearTimer(PlaybackTimer);
	PlaybackTimer.Invalidate();
}

void UShibMediaPlayer::PlaybackTick()
{
	const FTimespan TimeLeftThisTick = VideoDuration - GetTime();

	if (bRecentReset)
	{
		TimeLeft = TimeLeftThisTick;
		
		TicksAfterReset++;
		if (TicksAfterReset >= TicksToIgnoreAfterReset)
		{
			bRecentReset = false;
			TicksAfterReset = 0;
		}
		
		// UE_LOG(LogTemp, Log, TEXT("%hs - Waiting After Reset"), __FUNCTION__);
		return;
	}
	
	// if it's stuck but not buffering/connecting
	if (TimeLeftThisTick == TimeLeft && !IsPreparing() && !IsConnecting() && !IsPaused() && bIsWindowFocused)
	{
		// UE_LOG(LogTemp, Warning, TEXT("%hs - Stuck"), __FUNCTION__);
		VideoStuckTicks++;

		// if it's stuck x number of ticks, restart and continue from current time
		if (VideoStuckTicks >= VideoStuckThreshold)
		{
			UE_LOG(LogTemp, Warning, TEXT("%hs - Resetting"), __FUNCTION__);
			SeekTo = GetTime();
			TimeLeft = FTimespan::Zero();
			bRecentReset = true;
			VideoStuckTicks = 0;
			Reopen();
			return;
		}
	} else
	{
		// UE_LOG(LogTemp, Log, TEXT("%hs - Not Stuck"), __FUNCTION__);
		VideoStuckTicks = 0;
	}

	TimeLeft = TimeLeftThisTick;
}

void UShibMediaPlayer::OnMediaOpened_Implementation(FString OpenedUrl)
{
	// UE_LOG(LogTemp, Log, TEXT("%hs"), __FUNCTION__);
	VideoDuration = GetDuration();

	if (!SeekTo.IsZero())
	{
		UE_LOG(LogTemp, Log, TEXT("%hs - Seeking to %f"), __FUNCTION__, SeekTo.GetTotalSeconds());
		Seek(SeekTo);
		SeekTo = FTimespan::Zero();
	}

	Play();
}

void UShibMediaPlayer::OnMediaClosed_Implementation()
{
	// UE_LOG(LogTemp, Log, TEXT("%hs"), __FUNCTION__);
	StopTick();
}

void UShibMediaPlayer::OnPlaybackResumed_Implementation()
{
	// UE_LOG(LogTemp, Log, TEXT("%hs"), __FUNCTION__);
	if (!PlaybackTimer.IsValid())
	{
		StartTick();
	}
}

void UShibMediaPlayer::OnPlaybackSuspended_Implementation()
{
	// UE_LOG(LogTemp, Log, TEXT("%hs"), __FUNCTION__);
	StopTick();
}

void UShibMediaPlayer::OnEndReached_Implementation()
{
	// UE_LOG(LogTemp, Log, TEXT("%hs"), __FUNCTION__);
	StopTick();
}

void UShibMediaPlayer::OnSeekCompleted_Implementation()
{
	// UE_LOG(LogTemp, Log, TEXT("%hs"), __FUNCTION__);
}

void UShibMediaPlayer::OnFocusChanged(bool bFocus)
{
	// UE_LOG(LogTemp, Log, TEXT("%hs - Focus: %d"), __FUNCTION__, bFocus);
	bIsWindowFocused = bFocus;
}
