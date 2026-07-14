// Copyright Shiba Inu Games LLC.


#include "Streams/MediaStreamComponent.h"

#include "MediaSoundComponent.h"
#include "Streams/MediaStreamOrigin.h"
#include "Kismet/GameplayStatics.h"
#include "Streams/ShibMediaPlayer.h"
#include "Streams/ShibMediaStreamer.h"


UMediaStreamComponent::UMediaStreamComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMediaStreamComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!GetOwner()) return;

	// invalid components
	if (UniqueStreamerId < 0 || StreamerRatio == EStreamerRatio::SR_None) return;

	// get the origin to get the media
	TArray<AActor*> StreamOrigins;
	UGameplayStatics::GetAllActorsOfClass(this, AMediaStreamOrigin::StaticClass(), StreamOrigins);
	if (StreamOrigins.IsEmpty()) return;

	// make sure it's valid
	auto* StreamOrigin = Cast<AMediaStreamOrigin>(StreamOrigins[0]);
	if (!IsValid(StreamOrigin)) return;

	Streamer = StreamOrigin->AddComponent(GetStreamerId());

	if (VideoPlayerAudio && Streamer && Streamer->VideoPlayer)
	{
		VideoPlayerAudio->SetMediaPlayer(Streamer->VideoPlayer);
	}
}

FString UMediaStreamComponent::GetStreamerId()
{
	return FString::Printf(TEXT("%d-%d-%d"), StreamerLevel, StreamerRatio, UniqueStreamerId);
}
