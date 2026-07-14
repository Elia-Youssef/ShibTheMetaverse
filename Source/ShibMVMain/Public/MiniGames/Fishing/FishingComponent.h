// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "FishingBobber.h"
#include "Components/ActorComponent.h"
#include "MiniGames/ShibGameComponent.h"
#include "FishingComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShibItemPopUp, FName, RowName);

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHIBMVMAIN_API UFishingComponent : public UShibGameComponent
{
	GENERATED_BODY()

public:	
	UFishingComponent();
	
	UPROPERTY(ReplicatedUsing=OnRep_FishToCatch)
	FShibItem FishToCatch = FShibItem();
	
	UPROPERTY(ReplicatedUsing=OnRep_FishingLocation)
	FVector FishingLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintAssignable)
	FOnShibItemPopUp OnShibItemPopUp;
	
	UPROPERTY()
	FTimerHandle Timer_Fishing;

	UPROPERTY(BlueprintReadWrite)
	FFishingSpotData FishingSpotData;

	UPROPERTY()
	AFishingBobber* FishingIndicator = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> FishingHookClass;

	UPROPERTY(EditDefaultsOnly, Category="Fishing")
	float MinWaitingTime = 2.f;

	UPROPERTY(EditDefaultsOnly, Category="Fishing")
	float MaxWaitingTime = 10.f;

	UPROPERTY(EditDefaultsOnly, Category="Fishing")
	float UncommonFishChance = 30.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Fishing")
	float RareFishChance = 10.f;

	UPROPERTY(EditDefaultsOnly, Category="Fishing")
	float EpicFishChance = 5.f;

	/**This is a tech debt variable waiting for the animation to finish playing after catching a fish.*/
	UPROPERTY(EditDefaultsOnly, Category="Fishing")
	float FishingEndingDuration = 5.f;

	UPROPERTY(EditDefaultsOnly)
	FName ItemToConsume = "FishingString";

	UFUNCTION(BlueprintCallable)
	void InitializeFishingComponent(FVector HookPoint);
	
	UFUNCTION(BlueprintCallable)
	void AttemptToCatchFish();

	virtual bool CanPlayerStartGame() override;

protected:
	
	UFUNCTION()
	void OnCharacterMoved();
	
	virtual void DestroyComponent(bool bPromoteChildren) override;
	
	UFUNCTION()
	void OnRep_FishToCatch();

	UFUNCTION()
	void OnRep_FishingLocation();

	UFUNCTION()
	void StopFishing();

	UFUNCTION()
	void FishHooked();

	void RemoveFishingIndicator();

	UFUNCTION(Client, Reliable)
	void Client_ItemPopUp(FName RowName);
	
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	AFishingBobber* SpawnFishingHookPointAtLocation(const FVector& HookPoint) const;
	
	FShibItem GetRandomFishToCatch() const;
	
	FShibItem GetRandomFish() const;
	
	EShibItemTier DetermineFishTier() const;
	
};
