// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArcheryTarget.generated.h"

UCLASS()
class SHIBMVMAIN_API AArcheryTarget : public AActor
{
	GENERATED_BODY()
	
public:	
	AArcheryTarget();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 Score = 0;

	UFUNCTION(BlueprintImplementableEvent)
	void DestroyTarget();
	
protected:
	virtual void BeginPlay() override;
};
