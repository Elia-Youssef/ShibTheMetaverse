// Copyright Shiba Inu Games LLC.


#include "MiniGames/Fishing/FishingSpot.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MiniGames/Fishing/FishingSpotsOrigin.h"

AFishingSpot::AFishingSpot()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Static);
	SetRootComponent(Root);

	FishingZoneBox = CreateDefaultSubobject<UBoxComponent>("FishingZoneBox");
	FishingZoneBox->SetupAttachment(RootComponent);
	FishingZoneBox->SetMobility(EComponentMobility::Static);
	FishingZoneBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FishingZoneBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	FishingZoneBox->SetGenerateOverlapEvents(false);

	FishingInteractionBox = CreateDefaultSubobject<UBoxComponent>("FishingInteractionBox");
	FishingInteractionBox->SetupAttachment(RootComponent);
	FishingInteractionBox->SetMobility(EComponentMobility::Static);
	FishingInteractionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FishingInteractionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	FishingInteractionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	FishingInteractionBox->SetGenerateOverlapEvents(true);
}

FString AFishingSpot::GetSpotId()
{
	return FString::Printf(TEXT("%s_%d"), *UGameplayStatics::GetCurrentLevelName(this), UniqueId);
}

void AFishingSpot::BeginPlay()
{
	Super::BeginPlay();

	// get the actor or spawn it
	AFishingSpotsOrigin* Origin = nullptr;
	AActor* Actor = UGameplayStatics::GetActorOfClass(this, AFishingSpotsOrigin::StaticClass());
	if (Origin = Cast<AFishingSpotsOrigin>(Actor); !IsValid(Origin))
	{
		Origin = GetWorld()->SpawnActorDeferred<AFishingSpotsOrigin>(AFishingSpotsOrigin::StaticClass(), FTransform{});
	}

	if (!IsValid(Origin))
	{
		// if it's still not valid then something's wrong
		return;
	}

	Origin->OnSpotsDataUpdated.AddUniqueDynamic(this, &AFishingSpot::OnSpotsDataUpdated);
	Origin->AddSpot(GetSpotId());
}

void AFishingSpot::OnSpotsDataUpdated(const TArray<FFishingSpotData>& Data)
{
	for (auto& Spot : Data)
	{
		if (Spot.SpotId == GetSpotId())
		{
			SpotData = Spot;
			OnSpotDataUpdated();
			break;
		}
	}
}