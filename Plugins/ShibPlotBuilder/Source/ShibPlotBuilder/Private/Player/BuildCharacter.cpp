// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/BuildCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"


ABuildCharacter::ABuildCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 0.f);
		
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = true;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->DefaultLandMovementMode = MOVE_Flying;
	GetCharacterMovement()->MaxFlySpeed = 600.f;
	GetCharacterMovement()->BrakingDecelerationFlying = 4000.f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(GetRootComponent());
}

void ABuildCharacter::BeginPlay()
{
	Super::BeginPlay();
}
