// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseStreamActor.generated.h"

class UMediaSoundComponent;
class UMediaStreamComponent;

UCLASS()
class SHIBMVMAIN_API ABaseStreamActor : public AActor
{
	GENERATED_BODY()

public:
	ABaseStreamActor();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UMediaSoundComponent* VideoPlayerAudio;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UMediaStreamComponent* MediaStream;

protected:
	virtual void BeginPlay() override;
};
