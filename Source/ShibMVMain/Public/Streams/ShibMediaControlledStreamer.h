// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "Streams/ShibMediaStreamer.h"
#include "ShibMediaControlledStreamer.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class SHIBMVMAIN_API UShibMediaControlledStreamer : public UShibMediaStreamer
{
	GENERATED_BODY()

public:
	/**
	 * Same as parent, but takes the interactable media as an argument, which should be already cached
	 */
	void InitializeStreamer(UWorld* InWorld, const FString& InStreamerId, int32 InMaxCacheBufferSize, TArray<FCachedMedia>& InMedia);

	/**
	 * Manually skip forwards in the slideshow
	 */
	UFUNCTION(BlueprintCallable)
	void Next();

	/**
	 * Manually skip backwards in the slideshow
	 */
	UFUNCTION(BlueprintCallable)
	void Previous();

protected:
	/**
	 * Since this class will be manually controlled, and may move backwards,
	 * we'll need to cache media behind the current index.
	 */
	int32 CacheBufferOffset = 0;

	virtual TArray<int32> GetBufferIdx(TArray<FCachedMedia>& InMedia, int32 At = -1) override;
};
