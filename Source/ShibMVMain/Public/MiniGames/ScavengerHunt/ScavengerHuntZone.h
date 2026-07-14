// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ScavengerHuntComponent.h"
#include "GameFramework/Actor.h"
#include "ScavengerHuntZone.generated.h"

class UMetaverseApisSubsystem;

UCLASS()
class SHIBMVMAIN_API AScavengerHuntZone : public AActor
{
	GENERATED_BODY()

public:
	AScavengerHuntZone();

	UPROPERTY(EditDefaultsOnly)
	UDataTable* ItemsTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ZoneId = FName();

	UPROPERTY(BlueprintReadWrite)
	FScavengerHuntZoneDetails ZoneDetails;

	UPROPERTY(BlueprintReadWrite)
	TArray<AActor*> TargetSpawns = {};

protected:
	virtual void BeginPlay() override;

	void SpawnItems();
	FName GetRandomItemToSpawn();

	void GetDetailsFromServer();
	void SetTimerTillNextEvent();

private:
	UPROPERTY()
	UMetaverseApisSubsystem* MvApis;

	FTimerHandle ServerSyncHandle;
};
