// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibBaseGameState.h"
#include "Player/ShibPlayerState.h"
#include "ShibGameState.generated.h"

struct FChatMessage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerArrayChangeDelegate, APlayerState*, InPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUpdateChatDelegate);

UCLASS()
class SHIBMVMAIN_API AShibGameState : public AShibBaseGameState
{
	GENERATED_BODY()

public:
	AShibGameState();
	virtual void BeginPlay() override;

	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerArrayChangeDelegate OnPlayerArrayChangeDelegate;
	
	UFUNCTION(NetMulticast, Reliable)
	void Internal_OnPlayerJoin(APlayerState* InPlayerState);
	
	UFUNCTION(NetMulticast, Reliable)
	void Internal_OnPlayerLeave(APlayerState* InPlayerState);

	UFUNCTION(NetMulticast,Reliable)
	void AddChatMessage(const AShibPlayerState* ShibPS, const FString& Message);
	
	// Get all player locations in the current level
	UFUNCTION(BlueprintCallable, Category=PlayersInfo)
	TArray<FVector> GetAllPlayerLocations();
};
