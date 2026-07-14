// Copyright Shiba Inu Games LLC.


#include "MiniGames/ScavengerHunt/ScavengerHuntZone.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Metaverse/MetaverseApisSubsystem.h"
#include "MiniGames/ScavengerHunt/ScavengerHuntItem.h"
#include "MiniGames/ScavengerHunt/ScavengerHuntSpawnTarget.h"


AScavengerHuntZone::AScavengerHuntZone()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AScavengerHuntZone::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority() || ZoneId.IsNone()) return;

	UGameplayStatics::GetAllActorsOfClassWithTag(this, AScavengerHuntSpawnTarget::StaticClass(), ZoneId, TargetSpawns);

	if (auto* GI = GetGameInstance())
	{
		if (MvApis = GI->GetSubsystem<UMetaverseApisSubsystem>(); MvApis)
		{
			GetDetailsFromServer();
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindUObject(this, &ThisClass::GetDetailsFromServer);
			GetWorld()->GetTimerManager().SetTimer(ServerSyncHandle, TimerDelegate, 900.f, true);
		}
	}
}

void AScavengerHuntZone::SpawnItems()
{
	if (!ItemsTable || ZoneDetails.Items.IsEmpty()) return;

	for (const AActor* Target : TargetSpawns)
	{
		const auto* Row = ItemsTable->FindRow<FScavengerHuntItemDetails>(GetRandomItemToSpawn(), "");
		if (!Row)
		{
			continue;
		}

		auto* Actor = GetWorld()->SpawnActorDeferred<AScavengerHuntItem>(Row->ItemClass, Target->GetActorTransform());
		Actor->Details = *Row;
		Actor->FinishSpawning(Target->GetActorTransform());
	}
}

FName AScavengerHuntZone::GetRandomItemToSpawn()
{
	const int32 RandomIdx = FMath::RandRange(0, ZoneDetails.Items.Num() - 1);

	if (ZoneDetails.Items.IsValidIndex(RandomIdx))
	{
		return ZoneDetails.Items[RandomIdx];
	}

	return FName();
}

void AScavengerHuntZone::GetDetailsFromServer()
{
	MvApis->GetScavengerHuntDetails(ZoneId, [this](FScavengerHuntZoneDetails Details)
	{
		ZoneDetails = Details;
		SetTimerTillNextEvent();
	});
}

void AScavengerHuntZone::SetTimerTillNextEvent()
{
	FDateTime Now = FDateTime::Now();
	FTimespan ZoneTime;
	if (!FTimespan::Parse(ZoneDetails.Time, ZoneTime))
	{
		return;
	}

	FDateTime ZoneDateTime = FDateTime{
		Now.GetYear(), Now.GetMonth(), Now.GetDay(), ZoneTime.GetHours(), ZoneTime.GetMinutes(), ZoneTime.GetSeconds()
	};

	FTimespan DiffDateTime = UKismetMathLibrary::Subtract_DateTimeDateTime(ZoneDateTime, Now);
	float Diff = DiffDateTime.GetTotalSeconds();
	if (Diff > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(ServerSyncHandle, [this]()
		{
			SpawnItems();
		}, Diff, false);
	}
}
