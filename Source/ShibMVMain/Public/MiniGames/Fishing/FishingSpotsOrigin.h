// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibAPIsTypes.h"
#include "GameFramework/Actor.h"
#include "Inventory/ShibItemDataTable.h"
#include "Metaverse/MetaverseApisTypes.h"
#include "FishingSpotsOrigin.generated.h"

class UMetaverseApisSubsystem;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpotsDataUpdated, const TArray<FFishingSpotData>&, Data);

class UShibGameInstance;
class UShibAPIsSubsystem;

UCLASS()
class SHIBMVMAIN_API AFishingSpotsOrigin : public AActor
{
	GENERATED_BODY()

public:
	AFishingSpotsOrigin();

	void AddSpot(const FString& SpotId);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Server")
	int32 InitialDelay = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Server")
	int32 SyncRate = 10;

	UPROPERTY(BlueprintAssignable)
	FOnSpotsDataUpdated OnSpotsDataUpdated;

	UPROPERTY(ReplicatedUsing=OnRep_SpotsData)
	TArray<FFishingSpotData> SpotsData;
	UFUNCTION()
	void OnRep_SpotsData();

protected:
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeServerSync();

	UPROPERTY()
	TArray<FString> SpotsIds;

private:
	void GetSpotsData();

	UPROPERTY()
	UShibGameInstance* ShibGI = nullptr;

	UPROPERTY()
	UMetaverseApisSubsystem* MvAPIs = nullptr;

	FTimerHandle InitialDelayHandle;
	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;
};
