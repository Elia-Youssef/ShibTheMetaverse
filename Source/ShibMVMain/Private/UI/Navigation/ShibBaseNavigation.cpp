// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Navigation/ShibBaseNavigation.h"

#include "Kismet/GameplayStatics.h"
#include "Game/ShibGameInstance.h"
#include "Game/ShibGameState.h"
#include "Player/ShibBasePlayerController.h"
#include "Player/ShibPlayerState.h"

UShibGameInstance* UShibBaseNavigation::GetShibGI()
{
	if (!ShibGI) ShibGI = GetGameInstance<UShibGameInstance>();
	return ShibGI;
}

AShibBasePlayerController* UShibBaseNavigation::GetShibBaseController()
{
	if (!ShibBaseCtrl) ShibBaseCtrl = Cast<AShibBasePlayerController>(GetOwningPlayer());
	return ShibBaseCtrl;
}

AShibPlayerState* UShibBaseNavigation::GetShibPS()
{
	if (!ShibPS) ShibPS = GetOwningPlayerState<AShibPlayerState>();
	return ShibPS;
}

AShibGameState* UShibBaseNavigation::GetShibGS()
{
	if (!ShibGS) ShibGS = Cast<AShibGameState>(UGameplayStatics::GetGameState(GetOwningPlayer()));
	return ShibGS;
}
