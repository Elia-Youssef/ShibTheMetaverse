// Copyright Shiba Inu Games LLC.


#include "MiniGames/Fishing/FishingSpotsOrigin.h"

#include "Game/ShibGameInstance.h"
#include "Metaverse/MetaverseApisSubsystem.h"
#include "Net/UnrealNetwork.h"


AFishingSpotsOrigin::AFishingSpotsOrigin()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AFishingSpotsOrigin::BeginPlay()
{
	Super::BeginPlay();

	// Update the data on the server only then replicate
	if (!HasAuthority())
	{
		return;
	}

	ShibGI = GetGameInstance<UShibGameInstance>();
	if (!ShibGI) return;

	MvAPIs = ShibGI->GetSubsystem<UMetaverseApisSubsystem>();
	if (!MvAPIs) return;

	InitializeServerSync();
}

void AFishingSpotsOrigin::BeginDestroy()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(InitialDelayHandle);
	}

	Super::BeginDestroy();
}

void AFishingSpotsOrigin::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFishingSpotsOrigin, SpotsData);
}

void AFishingSpotsOrigin::AddSpot(const FString& SpotId)
{
	SpotsIds.AddUnique(SpotId);
}

void AFishingSpotsOrigin::InitializeServerSync()
{
	// wait a bit for all the components to be registered
	GetWorldTimerManager().SetTimer(InitialDelayHandle, [this]()
	{
		// if there are no components in the level
		if (SpotsIds.IsEmpty()) return;

		GetSpotsData();
		TimerDelegate.BindUObject(this, &ThisClass::GetSpotsData);
		GetWorldTimerManager().SetTimer(TimerHandle, TimerDelegate, SyncRate, true);
	}, InitialDelay, false);
}

void AFishingSpotsOrigin::GetSpotsData()
{
	if(MvAPIs)
	{
		MvAPIs->GetSpotsData
		(SpotsIds, [this](TArray<FFishingSpotData> Data)
			{
				SpotsData = Data;

				// TODO: NM_Standalone is for testing, remove later
				if (GetNetMode() == NM_Standalone || HasAuthority())
				{
					// update the data on the server for the fishing component
					OnRep_SpotsData();
				}
			}
		);
	}
}

void AFishingSpotsOrigin::OnRep_SpotsData()
{
	OnSpotsDataUpdated.Broadcast(SpotsData);
}
