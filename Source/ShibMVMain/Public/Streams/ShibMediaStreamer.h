// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "MediaStreamOrigin.h"
#include "Engine/Texture2DDynamic.h"
#include "Metaverse/MetaverseApisTypes.h"
#include "ShibMediaStreamer.generated.h"

class UShibMediaPlayer;
class UMediaSoundComponent;
class UMediaTexture;
class UMediaPlayer;
class UShibMediaControlledStreamer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlideshowStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlideshowStopped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMediaSync, bool, bEmptyMedia, bool, bEmptyInteractableMedia);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVideoChanged, UMediaTexture*, Texture, FString, Text, FString, TextAr);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnImageChanged, UTexture2DDynamic*, Texture, FString, Text, FString, TextAr);

USTRUCT(BlueprintType)
struct FCachedMedia : public FMedia
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2DDynamic* Texture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDownloadPending = false;

	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// video?

	static FCachedMedia New(const FMedia& Media)
	{
		return FCachedMedia{
			{
				Media.Id,
				Media.StreamerId,
				Media.MediaType,
				Media.URL,
				Media.SlideshowTime,
				Media.Text
			},
		};
	}

	/**
	 * Checks given array for a texture with this object's id and copies it
	 * @param Array Array to copy the texture from
	 * @return true if a valid texture is found and is copied
	 */
	bool CopyTextureFromArray(const TArray<FCachedMedia>& Array)
	{
		for (auto& i : Array)
		{
			if (Id == i.Id && IsValid(i.Texture))
			{
				bDownloadPending = i.bDownloadPending;
				Texture = i.Texture;
				return true;
			}
		}
		return false;
	}
};

/**
 * 
 */
UCLASS()
class SHIBMVMAIN_API UShibMediaStreamer : public UObject
{
	GENERATED_BODY()

protected:
	virtual UWorld* GetWorld() const override;

public:
	/**
	 * Sets the needed variables for this object
	 * @param InWorld World context
	 * @param InStreamerId This streamers unique Id
	 * @param InMaxCacheBufferSize How many images to cache
	 */
	void InitializeStreamer(UWorld* InWorld, const FString& InStreamerId, int32 InMaxCacheBufferSize);

	void DestroyStreamer();
	
	/**
	 * Starts caching textures for all locations and starts the slideshow
	 */
	UFUNCTION(BlueprintCallable)
	void InitializeSlideshow();

	/**
	 * Merges this object's media with the new array
	 */
	UFUNCTION(BlueprintCallable)
	void UpdateMedia(TArray<FMedia> NewMedia);

	/**
	 * Creates a new streamer with functions to control the slideshow
	 */
	UFUNCTION(BlueprintCallable)
	UShibMediaControlledStreamer* CreateInteractiveStreamer();

	/**
	 * Destroys the existing interactive streamer
	 */
	UFUNCTION(BlueprintCallable)
	void DestroyInteractiveStreamer();

	/**
	 * Is the streamer playing a slideshow or not
	 */
	UPROPERTY(BlueprintReadOnly)
	bool bIsPlaying = false;

	/**
	 * This streamer's unique Id
	 */
	UPROPERTY()
	FString StreamerId = FString();

	/**
	 * Video player to stream videos, created at runtime
	 */
	UPROPERTY(BlueprintReadOnly)
	UShibMediaPlayer* VideoPlayer = nullptr;

	/**
	 * Video player texture, set this in a material to view the stream
	 */
	UPROPERTY()
	UMediaTexture* VideoPlayerTexture = nullptr;

	/**
	 * How many images should we keep in memory?
	 * Higher -> high memory usage, low bandwidth usage
	 * Lower -> low memory usage, high bandwidth usage
	 */
	UPROPERTY()
	int32 MaxCacheBufferSize = 3;

	/**
	 * World context
	 */
	UPROPERTY()
	UWorld* World = nullptr;

	/**
	 * Called when the slideshow starts the first time
	 */
	UPROPERTY(BlueprintAssignable)
	FOnSlideshowStarted OnSlideshowStarted;

	/**
	 * Called every time a video or image ends, use this to add fade in/out
	 */
	UPROPERTY(BlueprintAssignable)
	FOnSlideshowStopped OnSlideshowStopped;
	
	/**
	 * Called when a video should start streaming, set the texture parameter in a material
	 */
	UPROPERTY(BlueprintAssignable)
	FOnVideoChanged OnVideoChanged;
	
	/**
	 * Called when the media list is updated from the database
	 */
	UPROPERTY(BlueprintAssignable)
	FOnMediaSync OnMediaSync;
	
	/**
	 * Called when an image should start streaming, set the texture parameter in a material
	 */
	UPROPERTY(BlueprintAssignable)
	FOnImageChanged OnImageChanged;

	UPROPERTY(BlueprintReadOnly)
	UShibMediaControlledStreamer* InteractiveStreamer;

protected:
	/**
	 * Current index of the slideshow based on `Media`
	 */
	UPROPERTY()
	int32 SlideshowIdx = 0;

	/**
	 * Used to set the slideshow time
	 */
	UPROPERTY()
	FTimerHandle SlideshowTimer;
	
	/**
	 * Used to set the delay timer before playing a video
	 */
	UPROPERTY()
	FTimerHandle VideoDelayHandle;

	/**
	 * The list of media to loop over,
	 * has cached textures based on the cache buffer size
	 */
	UPROPERTY()
	TArray<FCachedMedia> Media;

	/**
	 * The list of media to show on interaction,
	 * it is passed to a child actor `UShibMediaControlledStreamer`
	 */
	UPROPERTY()
	TArray<FCachedMedia> InteractableMedia;

	/**
	 * Returns the list of media indexes to cache based on the current slideshow index
	 */
	virtual TArray<int32> GetBufferIdx(TArray<FCachedMedia>& InMedia, int32 At = -1);
	
	/**
	 * Loops over the media array using the buffer size to cache images.
	 * if StartAtIdx < 0, it uses the slideshow index.
	 */
	void CacheTextures(TArray<FCachedMedia>& InMedia, int32 StartAtIdx = -1, const TFunction<void()>& OnFirstIndexReady = nullptr);

	UFUNCTION(BlueprintCallable)
	void StartSlideshow();
	void TickSlideshow();
	void NextSlideshow(float Delay = 0.f, int32 IncrementBy = 1);
	UFUNCTION(BlueprintCallable)
	void StopSlideshow();

	void StreamVideo(const FCachedMedia& Media);
	void StreamImage(const FCachedMedia& Media);

	UFUNCTION()
	void OnVideoPlayerOpened(FString Url);
	UFUNCTION()
	void OnVideoPlayerEnded();
	UFUNCTION()
	void OnVideoPlayerFailed(FString Url);
};
