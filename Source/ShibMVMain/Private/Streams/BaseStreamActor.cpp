// Copyright Shiba Inu Games LLC.


#include "Streams/BaseStreamActor.h"

#include "MediaSoundComponent.h"
#include "Streams/MediaStreamComponent.h"


ABaseStreamActor::ABaseStreamActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");
	RootComponent->SetMobility(EComponentMobility::Static);

	VideoPlayerAudio = CreateDefaultSubobject<UMediaSoundComponent>(TEXT("VideoPlayerAudio"));
	VideoPlayerAudio->SetupAttachment(GetRootComponent());

	MediaStream = CreateDefaultSubobject<UMediaStreamComponent>(TEXT("MediaStream"));
	MediaStream->VideoPlayerAudio = VideoPlayerAudio;
}

void ABaseStreamActor::BeginPlay()
{
	Super::BeginPlay();
}
