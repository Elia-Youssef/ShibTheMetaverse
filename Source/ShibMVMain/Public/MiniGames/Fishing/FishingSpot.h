// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "FishingSpotsOrigin.h"
#include "GameFramework/Actor.h"
#include "Utils/InteractInterface.h"
#include "FishingSpot.generated.h"

class UBoxComponent;

UCLASS()
class SHIBMVMAIN_API AFishingSpot : public AActor, public IInteractInterface
{
	GENERATED_BODY()

public:
	AFishingSpot();
	
	UFUNCTION(BlueprintPure)
	virtual FString GetSpotId();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USceneComponent* Root;

	/**
	 * This is the box to put in the lake.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* FishingZoneBox;

	/**
	 * This is the box the player will interact with to start fishing.
	 * It's only here for the interaction system to detect the spot.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* FishingInteractionBox;

	/**
	 * This will be populated from the database
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FFishingSpotData SpotData;

	/**
	 * Unique Id for this spot to enable/disable it from the database
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UniqueId = -1;
	
protected:
	virtual void BeginPlay() override;

	/**
	 * When all spots data gets updated
	 */
	UFUNCTION()
	void OnSpotsDataUpdated(const TArray<FFishingSpotData>& Data);
	
	/**
	 * When this spot's data gets updated
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void OnSpotDataUpdated();
};
