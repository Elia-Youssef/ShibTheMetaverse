// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShibChatComponent.generated.h"

class AShibPlayerState;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHIBMVMAIN_API UShibChatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UShibChatComponent();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SendChatMessage(const AShibPlayerState* ShibPS, const FString& Message);
};
