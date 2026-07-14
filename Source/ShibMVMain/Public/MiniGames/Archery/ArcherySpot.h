// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utils/InteractInterface.h"
#include "ArcherySpot.generated.h"

UCLASS()
class SHIBMVMAIN_API AArcherySpot : public AActor, public IInteractInterface
{
	GENERATED_BODY()
	
public:	
	AArcherySpot();

	virtual void Interact_Implementation(AActor* InteractionInstigator = nullptr) override;

protected:
	virtual void BeginPlay() override;
};
