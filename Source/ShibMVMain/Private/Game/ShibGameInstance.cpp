// Copyright Shib LLC.

#include "Game/ShibGameInstance.h"
#include "ShibAPIsSettings.h"
#include "ShibAPIsSubsystem.h"
#include "ShibAvatarBuilderSubsystem.h"
#include "ShibLoadingScreenManager.h"
#include "ShibSessionsEOS.h"
#include "Game/PlotBuilderSubsystem.h"
#include "Metaverse/MetaverseApisSubsystem.h"
#include "Utils/ShibFunctionLibrary.h"

void UShibGameInstance::Init()
{
	Super::Init();

	bIsGameError = false;
	// GetEngine()->OnNetworkFailure().AddUObject(this, &UShibGameInstance::HandleNetworkFailure);
	// GetEngine()->OnTravelFailure().AddUObject(this, &UShibGameInstance::HandleTravelFailure);

	AvatarBuildSubsystem = GetSubsystem<UShibAvatarBuilderSubsystem>();
	PlotBuilderSubsystem = GetSubsystem<UPlotBuilderSubsystem>();
	APIsSubsystem = GetSubsystem<UShibAPIsSubsystem>();
	MetaverseApisSubsystem = GetSubsystem<UMetaverseApisSubsystem>();

	if (IsDedicatedServerInstance())
	{
		auto* Settings = UShibAPIsSettings::Get();
		if (!Settings || Settings->DedicatedServerApiKey.IsEmpty()) return;
		UShibFunctionLibrary::SetCommandLineToken(Settings->DedicatedServerApiKey);
	} else
	{
		// this will be called again after login
		InitializeUser();
	}
}

void UShibGameInstance::InitializeUser()
{
	AvatarBuildSubsystem = GetSubsystem<UShibAvatarBuilderSubsystem>();
	PlotBuilderSubsystem = GetSubsystem<UPlotBuilderSubsystem>();
	APIsSubsystem = GetSubsystem<UShibAPIsSubsystem>();
	MetaverseApisSubsystem = GetSubsystem<UMetaverseApisSubsystem>();
	
	if (AvatarBuildSubsystem)
	{
		AvatarBuildSubsystem->OnAvatarBuilderSave.AddUniqueDynamic(this, &ThisClass::OnAvatarBuilderSave);
	}
	if (PlotBuilderSubsystem)
	{
		PlotBuilderSubsystem->OnPlotBuilderSave.AddUniqueDynamic(this, &ThisClass::OnPlotBuilderSave);
	}
	if (MetaverseApisSubsystem)
	{
		MetaverseApisSubsystem->IsAlwahaEnabled();
		MetaverseApisSubsystem->GetMyPlots();

		MetaverseApisSubsystem->OnGetMvPlayerDelegate.AddUniqueDynamic(this, &ThisClass::OnGetMvPlayer);
		MetaverseApisSubsystem->GetMvPlayer();
	}
}

void UShibGameInstance::Shutdown()
{
	if (auto* ShibEOS = GetSubsystem<UShibSessionsEOS>())
	{
		ShibEOS->CleanUpSessions();
	}
	
	if (auto* ShibAPIs = GetSubsystem<UShibAPIsSubsystem>(); ShibAPIs && !IsRunningDedicatedServer())
	{
		MetaverseApisSubsystem->Log(FShibLog{FString("shutdown"), FString()});
	}

	Super::Shutdown();
}

void UShibGameInstance::SetTravelInfo(const FName LevelKeyInfo,const FName TeleporterTagInfo)
{
	TeleporterTag = TeleporterTagInfo;

	// For showing custom loading screen, we save the map name
	if (UShibLoadingScreenManager* LoadingScreenManager = GetSubsystem<UShibLoadingScreenManager>())
	{
		LoadingScreenManager->SetDestinationMapName(LevelKeyInfo.ToString());
	}
}

void UShibGameInstance::ResetErrorWarnings()
{
	bIsGameError = false;
	GameErrorMsg = "";
}

void UShibGameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType,
                                             const FString& ErrorString)
{
	bIsGameError = true;
	GameErrorMsg = ErrorString;
}

void UShibGameInstance::HandleTravelFailure(UWorld* InWorld, ETravelFailure::Type FailureType,
                                            const FString& ErrorString)
{
	bIsGameError = true;
	GameErrorMsg = ErrorString;
}

void UShibGameInstance::OnPlotBuilderSave(const FString& Json)
{
	if (MetaverseApisSubsystem && SelectedPlotToView.PlotId != -1)
	{
		UE_LOG(LogTemp, Log, TEXT("[%hs] Json: %s"), __FUNCTION__, *Json);
		MetaverseApisSubsystem->PostPlotInfo(SelectedPlotToView.PlotId, Json);
	}
}

void UShibGameInstance::OnAvatarBuilderSave(const FString& Json)
{
	if (MetaverseApisSubsystem)
	{
		UE_LOG(LogTemp, Log, TEXT("[%hs] Json: %s"), __FUNCTION__, *Json);
		AvatarInfo.AvatarData = Json;
		MetaverseApisSubsystem->PostAvatarInfo(Json);
	}
}

void UShibGameInstance::OnGetMvPlayer(FShibMvPlayer Player, bool bSuccessful)
{
	if (bSuccessful)
	{
		AvatarInfo.AvatarData = Player.Avatar.AvatarData;
	}
}
