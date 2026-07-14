// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShibUiBaseNavigation.h"
#include "ShibBaseNavigation.generated.h"

class AShibGameState;
class AShibPlayerState;
class AShibBasePlayerController;
class UShibGameInstance;
/**
 * 
 */
UCLASS()
class SHIBMVMAIN_API UShibBaseNavigation : public UShibUiBaseNavigation
{
	GENERATED_BODY()
	
public:
	// ---- Getters ----
	
	UPROPERTY()
	TObjectPtr<UShibGameInstance> ShibGI;
	
	UFUNCTION(BlueprintPure)
	UShibGameInstance* GetShibGI();
	
	UPROPERTY()
	TObjectPtr<AShibBasePlayerController> ShibBaseCtrl;
	
	UFUNCTION(BlueprintPure)
	AShibBasePlayerController* GetShibBaseController();
	
	UPROPERTY()
	TObjectPtr<AShibPlayerState> ShibPS;
	
	UFUNCTION(BlueprintPure)
	AShibPlayerState* GetShibPS();
	
	UPROPERTY()
	TObjectPtr<AShibGameState> ShibGS;
	
	UFUNCTION(BlueprintPure)
	AShibGameState* GetShibGS();
	
};
