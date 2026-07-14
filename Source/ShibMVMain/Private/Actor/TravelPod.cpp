// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/TravelPod.h"
#include "Characters/ShibCharacterBase.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"

ATravelPod::ATravelPod()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>("Root");
	Root->SetMobility(EComponentMobility::Static);
	SetRootComponent(Root);

	SphereCollision = CreateDefaultSubobject<USphereComponent>("BoxCollision");
	SphereCollision->SetMobility(EComponentMobility::Static);
	SphereCollision->SetupAttachment(GetRootComponent());
	SphereCollision->CanCharacterStepUpOn = ECB_No;
}

void ATravelPod::BeginPlay()
{
	Super::BeginPlay();

	SphereCollision->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnBeginOverlap);
	SphereCollision->OnComponentEndOverlap.AddUniqueDynamic(this, &ThisClass::OnEndOverlap);
}

void ATravelPod::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bArrivalOnly) return;
	
	AShibCharacterBase* ShibCharacter = Cast<AShibCharacterBase>(OtherActor);
	if (ShibCharacter)
	{
		// Only detect collision with the characters capsule component
		if (OtherComp != ShibCharacter->GetCapsuleComponent()) return;
	}
	else return;
	
	// Don't overlap when character marked as teleporting
	if(ShibCharacter->GetIsTeleporting()) return;
	
	OnOverlap(true, ShibCharacter);
	ShibCharacter->OnTravelPodOverlap.Broadcast(true, Levels, LevelsToIgnore);
}

void ATravelPod::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (bArrivalOnly) return;

	AShibCharacterBase* ShibCharacter = Cast<AShibCharacterBase>(OtherActor);
	if (ShibCharacter)
	{
		// Only detect collision with the characters capsule component
		if (OtherComp != ShibCharacter->GetCapsuleComponent()) return;
	}
	else return;
	
	OnOverlap(false, ShibCharacter);
	ShibCharacter->OnTravelPodOverlap.Broadcast(false, Levels, LevelsToIgnore);
}
