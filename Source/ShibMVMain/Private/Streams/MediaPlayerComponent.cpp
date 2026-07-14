// Copyright Shiba Inu Games LLC.


#include "Streams/MediaPlayerComponent.h"

#include "MediaSoundComponent.h"
#include "Game/ShibGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Streams/ShibMediaControlledStreamer.h"
#include "Streams/ShibMediaPlayer.h"
#include "Metaverse/MetaverseApisSubsystem.h"


UMediaPlayerComponent::UMediaPlayerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMediaPlayerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!GetOwner()) return;
	
	ShibGI = Cast<UShibGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!ShibGI) return;

	MvAPIs = ShibGI->GetSubsystem<UMetaverseApisSubsystem>();
	if (!MvAPIs) return;

	if (!Streamer)
	{
		Streamer = NewObject<UShibMediaControlledStreamer>();
		Streamer->VideoPlayerTexture = VideoPlayerTexture;
		TArray<FCachedMedia> EmptyParam;
		Streamer->InitializeStreamer(GetWorld(), "local", 3, EmptyParam);
	}

	if (VideoPlayerAudio && Streamer && Streamer->VideoPlayer)
	{
		VideoPlayerAudio->SetMediaPlayer(Streamer->VideoPlayer);
	}
}
