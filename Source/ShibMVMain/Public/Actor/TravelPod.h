// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TravelPod.generated.h"

class AShibCharacterBase;
class USphereComponent;

UCLASS()
class SHIBMVMAIN_API ATravelPod : public AActor
{
	GENERATED_BODY()
	
public:	
	ATravelPod();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USphereComponent> SphereCollision;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintImplementableEvent)
	void OnOverlap(bool bBegin, AShibCharacterBase* OverlappingShibChar);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bArrivalOnly;

	// Leave this empty to travel anywhere
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Levels;
	
	// Filter out levels from the list above
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> LevelsToIgnore;
	
protected:
	void BeginPlay() override;
};
