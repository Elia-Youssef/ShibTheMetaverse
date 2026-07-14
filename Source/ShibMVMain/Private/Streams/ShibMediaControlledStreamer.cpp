// Copyright Shiba Inu Games LLC.


#include "Streams/ShibMediaControlledStreamer.h"

void UShibMediaControlledStreamer::InitializeStreamer(UWorld* InWorld, const FString& InStreamerId, int32 InMaxCacheBufferSize, TArray<FCachedMedia>& InMedia)
{
	Super::InitializeStreamer(InWorld, InStreamerId, InMaxCacheBufferSize);
	Media = InMedia;
	CacheBufferOffset = - InMaxCacheBufferSize / 2;
}

void UShibMediaControlledStreamer::Next()
{
	NextSlideshow(0.f, 1);
}

void UShibMediaControlledStreamer::Previous()
{
	NextSlideshow(0.f, -1);
}

TArray<int32> UShibMediaControlledStreamer::GetBufferIdx(TArray<FCachedMedia>& InMedia, int32 At)
{
	if (!InMedia.IsValidIndex(At)) At = SlideshowIdx;
	if (InMedia.IsEmpty()) return {};

	TArray<int32> Indexes;
	Indexes.AddUnique(At); // make sure the current index is the first in the array
	for (int32 i = 0; i < MaxCacheBufferSize; i++)
	{
		Indexes.AddUnique(FMath::Abs((At + i + CacheBufferOffset) % InMedia.Num()));
	}
	return Indexes;
}
