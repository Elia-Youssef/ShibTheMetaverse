// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "MiniGames/ShibGameComponent.h"
#include "ScavengerHuntComponent.generated.h"

struct FScavengerHuntItemDetails;
/**
 * 
 */
UCLASS()
class SHIBMVMAIN_API UScavengerHuntComponent : public UShibGameComponent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FScavengerHuntItemDetails> CollectedItems;
};
