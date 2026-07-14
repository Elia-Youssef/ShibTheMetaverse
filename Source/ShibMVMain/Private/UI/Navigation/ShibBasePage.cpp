// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Navigation/ShibBasePage.h"

#include "Characters/ShibCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Game/ShibGameInstance.h"
#include "Game/ShibGameState.h"
#include "Player/ShibBasePlayerController.h"
#include "Player/ShibPlayerState.h"

AShibCharacterBase* UShibBasePage::GetShibBaseCharacter()
{
	if (!ShibBaseCharacter && GetShibBaseController())
		ShibBaseCharacter = Cast<AShibCharacterBase>(GetShibBaseController()->GetPawn());
	return ShibBaseCharacter;
}

UShibGameInstance* UShibBasePage::GetShibGI()
{
	if (!ShibGI) ShibGI = GetGameInstance<UShibGameInstance>();
	return ShibGI;
}

AShibBasePlayerController* UShibBasePage::GetShibBaseController()
{
	if (!ShibBaseCtrl) ShibBaseCtrl = Cast<AShibBasePlayerController>(GetOwningPlayer());
	return ShibBaseCtrl;
}

AShibPlayerState* UShibBasePage::GetShibPS()
{
	if (!ShibPS) ShibPS = GetOwningPlayerState<AShibPlayerState>();
	return ShibPS;
}

AShibGameState* UShibBasePage::GetShibGS()
{
	if (!ShibGS) ShibGS = Cast<AShibGameState>(UGameplayStatics::GetGameState(GetOwningPlayer()));
	return ShibGS;
}
