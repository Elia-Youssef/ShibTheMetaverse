// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utils/InteractInterface.h"
#include "ScavengerHuntItem.generated.h"

class AScavengerHuntItem;

USTRUCT(BlueprintType)
struct FScavengerHuntItemDetails : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AScavengerHuntItem> ItemClass = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 Reward = 0;
};

class USphereComponent;

UCLASS()
class SHIBMVMAIN_API AScavengerHuntItem : public AActor, public IInteractInterface
{
	GENERATED_BODY()

public:
	AScavengerHuntItem();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USphereComponent* Sphere;

	UPROPERTY(BlueprintReadOnly)
	FScavengerHuntItemDetails Details;

protected:
	virtual void BeginPlay() override;
	virtual void Interact_Implementation(AActor* InteractionInstigator = nullptr) override;
};
