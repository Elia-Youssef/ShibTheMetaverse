// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShibBaseGameMode.generated.h"

class UShibGameInstance;

/**
 * This is the base GM that'll be used for single player levels
 * `AShibGameMode` will extend this class for multiplayer
 */
UCLASS()
class SHIBMVMAIN_API AShibBaseGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	AShibBaseGameMode();
	
	UFUNCTION(BlueprintCallable)
	void KickPlayer(APlayerController* Controller, FText KickReason) const;

	UFUNCTION(BlueprintPure)
	UShibGameInstance* GetShibGI();

	virtual void PreLogout(APlayerController* InPlayerController);

	virtual void Logout(AController* Exiting) override;

	bool CanRegisterPlayer(APlayerController* Exiting);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UShibGameInstance* ShibGI = nullptr;
};
