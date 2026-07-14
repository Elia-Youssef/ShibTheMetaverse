// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShibUiBasePage.h"
#include "ShibBaseNavigation.h"
#include "Player/ShibPlayerState.h"
#include "ShibBasePage.generated.h"

class AShibCharacterBase;
class AShibGameState;
class AShibPlayerState;
class AShibBasePlayerController;
class UShibGameInstance;
/**
 * 
 */
UCLASS()
class SHIBMVMAIN_API UShibBasePage : public UShibUiBasePage
{
	GENERATED_BODY()
	
public:
	// ---- Getters ----
	
	UFUNCTION(BlueprintPure)
	AShibCharacterBase* GetShibBaseCharacter();
	
	UFUNCTION(BlueprintPure)
	UShibGameInstance* GetShibGI();
	
	UFUNCTION(BlueprintPure)
	AShibBasePlayerController* GetShibBaseController();
	
	UFUNCTION(BlueprintPure)
	AShibPlayerState* GetShibPS();
	
	UFUNCTION(BlueprintPure)
	AShibGameState* GetShibGS();

private:
	UPROPERTY()
	TObjectPtr<AShibGameState> ShibGS;
	
	UPROPERTY()
	TObjectPtr<AShibPlayerState> ShibPS;
	
	UPROPERTY()
	TObjectPtr<AShibBasePlayerController> ShibBaseCtrl;
	
	UPROPERTY()
	TObjectPtr<UShibGameInstance> ShibGI;
	
	UPROPERTY()
	TObjectPtr<AShibCharacterBase> ShibBaseCharacter;
};
