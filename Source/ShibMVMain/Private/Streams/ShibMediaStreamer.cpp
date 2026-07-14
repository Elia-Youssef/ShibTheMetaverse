// Copyright Shiba Inu Games LLC.


#include "Streams/ShibMediaStreamer.h"

#include "MediaTexture.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Streams/DownloadImageWrapper.h"
#include "Streams/ShibMediaControlledStreamer.h"
#include "Streams/ShibMediaPlayer.h"


UWorld* UShibMediaStreamer::GetWorld() const
{
	return World;
}

void UShibMediaStreamer::InitializeStreamer(UWorld* InWorld, const FString& InStreamerId, int32 InMaxCacheBufferSize)
{
	World = InWorld;
	StreamerId = InStreamerId;
	MaxCacheBufferSize = InMaxCacheBufferSize;

	if (!VideoPlayer)
	{
		VideoPlayer = NewObject<UShibMediaPlayer>();
		VideoPlayer->Initialize(GetWorld());
	}
	VideoPlayer->PlayOnOpen = false;

	if (!VideoPlayerTexture)
	{
		VideoPlayerTexture = NewObject<UMediaTexture>();
	}
	VideoPlayerTexture->NewStyleOutput = true;
	VideoPlayerTexture->SetMediaPlayer(VideoPlayer);
	VideoPlayerTexture->UpdateResource();
}

void UShibMediaStreamer::DestroyStreamer()
{
	bIsPlaying = false;

	if (IsValid(VideoPlayer))
	{
		VideoPlayer->Close();
	}
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(SlideshowTimer);
		GetWorld()->GetTimerManager().ClearTimer(VideoDelayHandle);
	}

	DestroyInteractiveStreamer();

	OnImageChanged.Clear();
	OnVideoChanged.Clear();
	OnSlideshowStarted.Clear();
	OnSlideshowStopped.Clear();
}

UShibMediaControlledStreamer* UShibMediaStreamer::CreateInteractiveStreamer()
{
	if (IsValid(InteractiveStreamer))
	{
		return InteractiveStreamer;
	}

	InteractiveStreamer = NewObject<UShibMediaControlledStreamer>();
	InteractiveStreamer->InitializeStreamer(GetWorld(), StreamerId, MaxCacheBufferSize, InteractableMedia);
	return InteractiveStreamer;
}

void UShibMediaStreamer::DestroyInteractiveStreamer()
{
	if (!IsValid(InteractiveStreamer))
	{
		return;
	}
	InteractableMedia = InteractiveStreamer->Media; // take the cached media before destroying
	InteractiveStreamer->DestroyStreamer();
	InteractiveStreamer = nullptr;
}

void UShibMediaStreamer::UpdateMedia(TArray<FMedia> NewMedia)
{
	TArray<FCachedMedia> NewCachedMedia;
	TArray<FCachedMedia> NewCachedInteractableMedia;

	for (auto& M : NewMedia)
	{
		// skip other media
		if (M.StreamerId != StreamerId) continue;

		auto CachedM = FCachedMedia{M};
		if (M.Interactable)
		{
			CachedM.CopyTextureFromArray(InteractableMedia);
			NewCachedInteractableMedia.AddUnique(CachedM);
		}
		else
		{
			CachedM.CopyTextureFromArray(Media);
			NewCachedMedia.AddUnique(CachedM);
		}
	}
	Media = NewCachedMedia;
	InteractableMedia = NewCachedInteractableMedia;

	if (InteractiveStreamer)
	{
		InteractiveStreamer->Media = InteractableMedia;
		InteractiveStreamer->CacheTextures(InteractiveStreamer->Media);
	}
	else
	{
		// cache interactable media starting at idx 0
		CacheTextures(InteractableMedia, 0);
	}

	OnMediaSync.Broadcast(Media.IsEmpty(), InteractableMedia.IsEmpty());
}

void UShibMediaStreamer::InitializeSlideshow()
{
	if ((Media.IsEmpty() && InteractableMedia.IsEmpty()) || bIsPlaying) return;

	StopSlideshow();
	CacheTextures(Media, -1, [this]()
	{
		// once we have the main images cached, cache interactable media
		CacheTextures(InteractableMedia);
		StartSlideshow();
	});
}

TArray<int32> UShibMediaStreamer::GetBufferIdx(TArray<FCachedMedia>& InMedia, int32 At)
{
	if (!InMedia.IsValidIndex(At)) At = SlideshowIdx;
	if (InMedia.IsEmpty()) return {};

	TArray<int32> Indexes;
	Indexes.AddUnique(At); // make sure the current index is the first in the array
	for (int32 i = 0; i < MaxCacheBufferSize; i++)
	{
		Indexes.AddUnique(FMath::Abs((At + i) % InMedia.Num()));
	}
	return Indexes;
}

void UShibMediaStreamer::CacheTextures(TArray<FCachedMedia>& InMedia, const int32 StartAtIdx,
                                       const TFunction<void()>& OnFirstIndexReady)
{
	TArray<int32> Indexes = GetBufferIdx(InMedia, StartAtIdx);
	if (Indexes.IsEmpty()) return;

	// loop over the buffer and cache
	for (int32 i : Indexes)
	{
		// if it's not an image, skip
		if (InMedia[i].MediaType != EMediaType::MT_Image)
		{
			if (i == Indexes[0] && OnFirstIndexReady != nullptr)
			{
				OnFirstIndexReady();
			}
			continue;
		}

		// if the image is cached, skip
		if (IsValid(InMedia[i].Texture))
		{
			if (i == Indexes[0] && OnFirstIndexReady != nullptr)
			{
				OnFirstIndexReady();
			}
			continue;
		}

		// if we're already downloading this one, skip
		if (InMedia[i].bDownloadPending)
		{
			continue;
		}

		// download the image
		UDownloadImageWrapper* Download = NewObject<UDownloadImageWrapper>();
		FOnImageDownloaded OnImageDownloaded;
		OnImageDownloaded.AddLambda(
			[&InMedia, Indexes, i, OnFirstIndexReady](UTexture2DDynamic* Texture)
			{
				InMedia[i].bDownloadPending = false;
				InMedia[i].Texture = Texture;

				if (i == Indexes[0] && OnFirstIndexReady != nullptr)
				{
					OnFirstIndexReady();
				}
			});

		InMedia[i].bDownloadPending = true;
		Download->Start(InMedia[i].URL, OnImageDownloaded);
	}
}

void UShibMediaStreamer::StartSlideshow()
{
	if (IsValid(VideoPlayer))
	{
		VideoPlayer->OnMediaOpened.AddUniqueDynamic(this, &ThisClass::OnVideoPlayerOpened);
		VideoPlayer->OnEndReached.AddUniqueDynamic(this, &ThisClass::OnVideoPlayerEnded);
		VideoPlayer->OnMediaOpenFailed.AddUniqueDynamic(this, &ThisClass::OnVideoPlayerFailed);
		VideoPlayer->Seek(0.f);
	}

	StopSlideshow();
	OnSlideshowStarted.Broadcast();
	bIsPlaying = true;
	TickSlideshow();
}

void UShibMediaStreamer::TickSlideshow()
{
	if (!bIsPlaying) return;

	if (Media.Num() <= 0)
	{
		StopSlideshow();
		return;
	}

	CacheTextures(Media);

	if (!Media.IsValidIndex(SlideshowIdx))
	{
		NextSlideshow(2.f);
		return;
	}
	FCachedMedia M = Media[SlideshowIdx];

	switch (M.MediaType)
	{
	case EMediaType::MT_Video:
		StreamVideo(M);
		return;
	case EMediaType::MT_Image:
		StreamImage(M);
		return;
	default:
		break;
	}

	NextSlideshow(2.f);
}

void UShibMediaStreamer::NextSlideshow(float Delay, int32 IncrementBy)
{
	if (IsValid(VideoPlayer))
	{
		VideoPlayer->Close();
		VideoPlayer->Seek(0.f);
	}

	if (Media.IsEmpty()) return;

	GetWorld()->GetTimerManager().ClearTimer(SlideshowTimer);

	if (Delay > 0.0f)
	{
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(SlideshowTimer, [this, IncrementBy]()
			{
				SlideshowIdx = FMath::Abs((SlideshowIdx + IncrementBy) % Media.Num());
				TickSlideshow();
			}, Delay, false);
		}
	}
	else
	{
		SlideshowIdx = FMath::Abs((SlideshowIdx + IncrementBy) % Media.Num());
		TickSlideshow();
	}
}

void UShibMediaStreamer::StopSlideshow()
{
	bIsPlaying = false;

	if (IsValid(VideoPlayer))
	{
		VideoPlayer->Close();
	}

	SlideshowIdx = 0;
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(SlideshowTimer);
	}

	OnSlideshowStopped.Broadcast();
}

void UShibMediaStreamer::StreamVideo(const FCachedMedia& CachedMedia)
{
	if (!IsValid(VideoPlayer)) return;

	// check if we can play this URL, skip if not
	if (!VideoPlayer->CanPlayUrl(CachedMedia.URL))
	{
		NextSlideshow(2.f);
		return;
	}

	VideoPlayer->OpenUrl(CachedMedia.URL);
}

void UShibMediaStreamer::StreamImage(const FCachedMedia& CachedMedia)
{
	if (CachedMedia.SlideshowTime <= 0.f)
	{
		NextSlideshow(2.f);
		return;
	}
	OnImageChanged.Broadcast(CachedMedia.Texture, CachedMedia.Text, CachedMedia.TextAr);
	NextSlideshow(CachedMedia.SlideshowTime);
}

void UShibMediaStreamer::OnVideoPlayerOpened(FString Url)
{
	GetWorld()->GetTimerManager().SetTimer(VideoDelayHandle, [this]()
	{
		if (IsValid(VideoPlayer))
		{
			VideoPlayer->Play();
			OnVideoChanged.Broadcast(VideoPlayerTexture, Media[SlideshowIdx].Text, Media[SlideshowIdx].TextAr);
		}
	}, 1.0, false);
}

void UShibMediaStreamer::OnVideoPlayerEnded()
{
	NextSlideshow(1.f);
}

void UShibMediaStreamer::OnVideoPlayerFailed(FString Url)
{
	NextSlideshow(2.f);
}
