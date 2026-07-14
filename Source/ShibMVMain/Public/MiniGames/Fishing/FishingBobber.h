// Copyright Shiba Inu Games LLC.

#pragma once

#include "FishingBobber.generated.h"

class UBoxComponent;

UCLASS()
class SHIBMVMAIN_API AFishingBobber : public AActor
{
	GENERATED_BODY()

public:
	AFishingBobber();

	void SetMaxWaitingTime(float NewTime);
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnFishHooked();

	UFUNCTION(BlueprintImplementableEvent)
	void OnCountdownReady(float MaxWaitingTime);
	
};
