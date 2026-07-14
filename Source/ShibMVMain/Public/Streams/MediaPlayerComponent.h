// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Metaverse/MetaverseApisTypes.h"
#include "MediaPlayerComponent.generated.h"


class UMediaTexture;
class UMetaverseApisSubsystem;
class UShibGameInstance;
class UShibMediaControlledStreamer;
class UMediaSoundComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SHIBMVMAIN_API UMediaPlayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMediaPlayerComponent();

	UPROPERTY(BlueprintReadOnly)
	UShibMediaControlledStreamer* Streamer = nullptr;

	UPROPERTY(BlueprintReadWrite)
	UMediaSoundComponent* VideoPlayerAudio = nullptr;
	
	UPROPERTY(EditDefaultsOnly)
	UMediaTexture* VideoPlayerTexture = nullptr;
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY()
	TArray<FMedia> MediaArray;

private:
	UPROPERTY()
	UShibGameInstance* ShibGI = nullptr;

	UPROPERTY()
	UMetaverseApisSubsystem* MvAPIs = nullptr;
};
