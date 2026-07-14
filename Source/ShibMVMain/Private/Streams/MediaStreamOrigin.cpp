// Copyright Shiba Inu Games LLC.


#include "Streams/MediaStreamOrigin.h"
#include "Streams/ShibMediaStreamer.h"
#include "Game/ShibGameInstance.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Metaverse/MetaverseApisSubsystem.h"


AMediaStreamOrigin::AMediaStreamOrigin()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AMediaStreamOrigin::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		Server_GetMedia();
		return;
	}

	// make sure the game instance is valid
	ShibGI = GetGameInstance<UShibGameInstance>();
	if (!ShibGI) return;

	// make sure the apis subsystem is valid
	MvAPIs = ShibGI->GetSubsystem<UMetaverseApisSubsystem>();
	if (!MvAPIs) return;

	InitializeServerSync();
}

void AMediaStreamOrigin::BeginDestroy()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(InitialDelayHandle);
	}

	for (auto& [Key, Streamer] : Streamers)
	{
		if (IsValid(Streamer))
		{
			Streamer->DestroyStreamer();
		}
	}
	Streamers.Empty();

	Super::BeginDestroy();
}

void AMediaStreamOrigin::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMediaStreamOrigin, MediaArray);
}

void AMediaStreamOrigin::InitializeServerSync() // server
{
	// wait a bit for all the components to be registered
	GetWorldTimerManager().SetTimer(InitialDelayHandle, [this]()
	{
		// if there are no components in the level
		if (Streamers.IsEmpty()) return;

		GetMediaFromServer();
		TimerDelegate.BindUObject(this, &ThisClass::GetMediaFromServer);
		GetWorldTimerManager().SetTimer(TimerHandle, TimerDelegate, SyncRate, true);
	}, InitialDelay, false);
}

UShibMediaStreamer* AMediaStreamOrigin::AddComponent(FString StreamerId) // server & client
{
	if (Streamers.Contains(StreamerId))
	{
		return Streamers[StreamerId];
	}

	UShibMediaStreamer* Streamer = NewObject<UShibMediaStreamer>();
	Streamer->InitializeStreamer(GetWorld(), StreamerId, MaxCacheBufferSize);
	Streamers.Add(StreamerId, Streamer);

	return Streamer;
}

void AMediaStreamOrigin::GetMediaFromServer() // server
{
	MvAPIs->OnGetMediaDelegate.AddUniqueDynamic(this, &ThisClass::OnGetMediaFromServer);
	// get all media for the Media Ids in this level
	TArray<FString> StreamerIds;
	Streamers.GetKeys(StreamerIds);
	MvAPIs->GetMedia(StreamerIds);
}

void AMediaStreamOrigin::OnGetMediaFromServer(const TArray<FMedia>& Media, bool bSuccessful) // server
{
	if (!bSuccessful) return;
	MediaArray = Media; // replicates to clients

	// if this is single player, manually "rep" it
	if (GetNetMode() == NM_Standalone)
	{
		OnRep_MediaArray();
	}
}

void AMediaStreamOrigin::OnRep_MediaArray() // client
{
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Media Length: %d"), MediaArray.Num()), true, true,
	                                  FLinearColor::Blue, 20.f);
	for (auto& [Location, Streamer] : Streamers)
	{
		Streamer->UpdateMedia(MediaArray);
		Streamer->InitializeSlideshow();
	}
}

void AMediaStreamOrigin::Server_GetMedia_Implementation()
{
	Client_GetMedia(MediaArray);
}

void AMediaStreamOrigin::Client_GetMedia_Implementation(const TArray<FMedia>& Media)
{
	MediaArray = Media;
	OnRep_MediaArray();
}
